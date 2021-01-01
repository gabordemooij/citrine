#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <sys/wait.h>
#include <syslog.h>
#include <signal.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef forLinux
#include <bsd/stdlib.h>
#include <bsd/string.h>
#endif

#include "citrine.h"
#include "siphash.h"

int ctr_gc_dust_counter;
int ctr_gc_object_counter;
int ctr_gc_kept_counter;
int ctr_gc_sticky_counter;
int ctr_gc_mode;

// call this function to start a nanosecond-resolution timer
struct timespec timer_start(){
    struct timespec start_time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
    return start_time;
}

// call this function to end a timer, returning nanoseconds elapsed as a long
long timer_end(struct timespec start_time){
    struct timespec end_time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
    long diffInNanos = (end_time.tv_sec - start_time.tv_sec) * (long)1e9 + (end_time.tv_nsec - start_time.tv_nsec);
    return diffInNanos;
}

int CtrTimerStartEnd = 0;

struct timespec vartime;
long time_elapsed_nanos;

/**
 * @internal
 * GarbageCollector Marker
 */
void ctr_gc_mark(ctr_object* object) {
	ctr_object* el;
	ctr_mapitem* item;
	ctr_object* o;
	ctr_object* k;
	long i;
	if (object->info.type == CTR_OBJECT_TYPE_OTARRAY) {
		for (i = 0; i < object->value.avalue->head; i++) {
			el = *(object->value.avalue->elements+i);
			if (!el->info.mark) {
				el->info.mark = 1;
				ctr_gc_mark(el);
			}
		}
	}
	item = object->properties->head;
	while(item) {
		k = item->key;
		o = item->value;
		k->info.mark = 1;
		if (!o->info.mark) {
			o->info.mark = 1;
			ctr_gc_mark(o);
		}
		item = item->next;
	} 
	item = object->methods->head;
	while(item) {
		o = item->value;
		k = item->key;
		k->info.mark = 1;
		if (!o->info.mark) {
			o->info.mark = 1;
			ctr_gc_mark(o);
		}
		item = item->next;
	} 
}

/**
 * @internal
 * GarbageCollector Sweeper
 */
void ctr_gc_sweep( int all ) {
	ctr_object* previousObject = NULL;
	ctr_object* currentObject = ctr_first_object;
	ctr_object* nextObject = NULL;
	ctr_mapitem* mapItem = NULL;
	ctr_mapitem* tmp = NULL;
	while(currentObject) {
		ctr_gc_object_counter ++;
		if ( ( currentObject->info.mark==0 && currentObject->info.sticky==0 ) || all){
			ctr_gc_dust_counter ++;
			/* remove from linked list */
			if (previousObject) {
				if (currentObject->gnext) {
					previousObject->gnext = currentObject->gnext;
					nextObject = currentObject->gnext;
				} else {
					previousObject->gnext = NULL;
					nextObject = NULL;
				}
			} else {
				if (currentObject->gnext) {
					ctr_first_object = currentObject->gnext;
					nextObject = currentObject->gnext;
				} else {
					ctr_first_object = NULL;
					nextObject = NULL;
				}
			}
			if (currentObject->methods->head) {
				mapItem = currentObject->methods->head;
				while(mapItem) {
					tmp = mapItem->next;
					ctr_heap_free( mapItem );
					mapItem = tmp;
				}
			}
			if (currentObject->properties->head) {
				mapItem = currentObject->properties->head;
				while(mapItem) {
					tmp = mapItem->next;
					ctr_heap_free( mapItem );
					mapItem = tmp;
				}
			}
			ctr_heap_free( currentObject->methods );
			ctr_heap_free( currentObject->properties );
			switch (currentObject->info.type) {
				case CTR_OBJECT_TYPE_OTSTRING:
					if (currentObject->value.svalue != NULL) {
						if (currentObject->value.svalue->vlen > 0) {
							ctr_heap_free( currentObject->value.svalue->value );
						}
						ctr_heap_free( currentObject->value.svalue );
					}
				break;
				case CTR_OBJECT_TYPE_OTARRAY:
					ctr_heap_free( currentObject->value.avalue->elements );
					ctr_heap_free( currentObject->value.avalue );
				break;
				case CTR_OBJECT_TYPE_OTEX:
					if (currentObject->value.rvalue != NULL) ctr_heap_free( currentObject->value.rvalue );
				break;
			}
			ctr_heap_free( currentObject );
			currentObject = nextObject;
		} else {
			ctr_gc_kept_counter ++;
			if (currentObject->info.sticky==1) ctr_gc_sticky_counter++;
			if (currentObject->info.mark == 1) {
				currentObject->info.mark = 0;
			}
			previousObject = currentObject;
			currentObject = currentObject->gnext;
		}
	}
}

/**
 * @internal
 * Garbage Collector sweep.
 */
void  ctr_gc_internal_collect() {
	ctr_object* context;
	int oldcid;
	ctr_gc_dust_counter = 0;
	ctr_gc_object_counter = 0;
	ctr_gc_kept_counter = 0;
	ctr_gc_sticky_counter = 0;
	context = ctr_contexts[ctr_context_id];
	oldcid = ctr_context_id;
	while(ctr_context_id > -1) {
		ctr_gc_mark(context);
		ctr_context_id--;
		context = ctr_contexts[ctr_context_id];
	}
	ctr_gc_sweep( 0 );
	ctr_context_id = oldcid;
}

ctr_object* ctr_gc_internal_pin( ctr_object* object ) {
	ctr_object* key = ctr_build_empty_string();
	ctr_internal_object_add_property( ctr_contexts[ctr_context_id], key, object, CTR_CATEGORY_PRIVATE_PROPERTY );
	return key;
}
 
/**
 * @def
 * [ Program ] clean memory
 *
 * @example
 * ☞ x ≔ 123.
 * ✎ write: Program memory, stop.
 * Program clean memory.
 * ✎ write: Program memory, stop.
 */
ctr_object* ctr_gc_collect (ctr_object* myself, ctr_argument* argumentList) {
	ctr_gc_internal_collect(); /* calls internal because automatic GC has to use this function as well and requires low overhead. */
	return myself;
}

/**
 * [Program] memory
 * 
 * Returns memory statistics.
 * Array with:
 * 0: allocated memory
 * 1: number of objects
 * 2: number of sticky objects
 * 3: number of remaining objects
 * 4: number of removed objects
 *
 * In other languages:
 * Dutch: Programma geheugen.
 * Geeft informatie over geheugengebruik terug.
 * Geeft reeks met op positie
 * 0: gealloceerde ruimte,
 * 1: aantal objecten
 * 2: aantal vastgezette objecten
 * 3: aantal overgebleven objecten
 * 4: aantal opgeruimde objecten 
 */
ctr_object* ctr_gc_memory(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* list = ctr_array_new( CtrStdArray, NULL );
	ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
	args->object = ctr_build_number_from_float( ctr_gc_alloc );
	ctr_array_push( list, args );
	args->object = ctr_build_number_from_float( ctr_gc_object_counter );
	ctr_array_push( list, args );
	args->object = ctr_build_number_from_float( ctr_gc_sticky_counter );
	ctr_array_push( list, args );
	args->object = ctr_build_number_from_float( ctr_gc_kept_counter );
	ctr_array_push( list, args );
	args->object = ctr_build_number_from_float( ctr_gc_dust_counter );
	ctr_array_push( list, args );
	ctr_heap_free(args);
	return list;
}

/**
 * [Program] memory: [Number]
 *
 * Sets the memory limit, if this limit gets exceeded the program will produce
 * an out-of-memory error.
 * 
 * In other languages:
 * Dutch: Programma geheugen: [getal].
 * Stelt geheugenlimiet in, in bytes.
 */
ctr_object* ctr_gc_setmemlimit(ctr_object* myself, ctr_argument* argumentList) {
	ctr_gc_memlimit = (uint64_t) ctr_internal_cast2number( argumentList->object )->value.nvalue;
	return myself;
}

/**
 * [Program] tidiness: [Number]
 *
 * Selects mode of operation for GC.
 *
 * Available Modes:
 * 0 - No Garbage Collection
 * 1 - Activate Garbage Collector (default)
 * 4 - Activate Garbage Collector for every single step (testing only)
 * 8 - Activate experimental Pool Memory Allocation Manager (experimental!)
 * 
 * In other languages:
 * Dutch: Programma opruimingsniveau: [getal].
 *
 * Mogelijke instellingen:
 * 0 - Geen opruiming
 * 1 - Standaard opruiming
 * 4 - Geheugen opruimen bij elke stap in het programma
 * 8 - Gebruik maken van gedeelde geheugen blokken (experimenteel)
 */
ctr_object* ctr_gc_setmode(ctr_object* myself, ctr_argument* argumentList) {
	ctr_gc_mode = (int) ctr_internal_cast2number( argumentList->object )->value.nvalue;
	if (ctr_gc_mode & 8) {
		ctr_pool_init(ctr_gc_memlimit/2);
	}
	return myself;
}

/**
 * @def
 * [ Program ] shell: [ String ]
 * 
 * @example
 * ☞ x ≔ Program shell: Command date.
 * ✎ write: x.
 */
ctr_object* ctr_program_shell(ctr_object* myself, ctr_argument* argumentList) {
	FILE* stream;
	char* outputBuffer;
	ctr_argument* newArgumentList;
	ctr_object* appendString;
	ctr_object* outputString;
	outputBuffer = ctr_heap_allocate( 512 );
	ctr_object* arg = ctr_internal_cast2string(argumentList->object);
	long vlen = arg->value.svalue->vlen;
	char* comString = ctr_heap_allocate( sizeof( char ) * ( vlen + 1 ) );
	memcpy(comString, arg->value.svalue->value, vlen);
	memcpy(comString+vlen,"\0",1);
	newArgumentList = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	if ( !( stream = popen( comString, "r" ) ) ) {
		CtrStdFlow = ctr_error( CTR_ERR_EXEC, 0 );
	}
	outputString = ctr_build_empty_string();
	while ( fgets( outputBuffer, 512, stream ) ) {
		appendString = ctr_build_string_from_cstring( outputBuffer );
		newArgumentList->object = appendString;
		ctr_string_append( outputString, newArgumentList );
	}
	pclose( stream );
	ctr_heap_free( outputBuffer );
	ctr_heap_free( newArgumentList );
	ctr_heap_free( comString );
	return outputString;
}

/**
 * @def
 * [ Program ] argument: [ Number ]
 *
 * @example
 * ☞ x ≔ Program argument: 1.
 * ✎ write: x, stop.
 */
ctr_object* ctr_program_argument(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* numberObject = ctr_internal_cast2number(argumentList->object);
	int n = (int) numberObject->value.nvalue;
	if (n > ctr_argc || n < 1) return CtrStdNil;
	return ctr_build_string(ctr_argv[n-1], strlen(ctr_argv[n-1]));
}

/**
 * @def
 * [ Program ] number.
 *
 * @example
 * ☞ x ≔ Program number.
 * ✎ write: x, stop.
 */
ctr_object* ctr_program_tonumber(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(CTR_VERSION_NUM);
}

/**
 * @def
 * [ Program ] string.
 *
 * @example
 * ☞ x ≔ Program string.
 * ✎ write: x, stop.
 */
ctr_object* ctr_program_tostring(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_string_from_cstring(CTR_MSG_WELCOME);
}


/**
 * @def
 * [ Program ] use: [ String ]
 *
 * @example
 * ☞ f ≔ File new: (Path /tmp: ‘x.ctr’).
 * f write: ‘☞ x ≔ 123.’.
 * Program use: (Path /tmp: ‘x.ctr’).
 * ✎ write: x, stop.
 */
ctr_object* ctr_program_include(ctr_object* myself, ctr_argument* argumentList) {
	ctr_size vlen;
	char* pathString;
	ctr_object* path;
	ctr_tnode* parsedCode;
	char* prg;
	uint64_t program_size;
	path = ctr_internal_cast2string(argumentList->object);
	vlen = path->value.svalue->vlen;
	pathString = ctr_heap_allocate_tracked(sizeof(char)*(vlen+1)); //needed until end, pathString appears in stracktrace
	if (path == NULL) return myself;
	program_size = 0;
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	prg = ctr_internal_readf(pathString, &program_size);
	parsedCode = ctr_cparse_parse(prg, pathString);
	if (parsedCode == NULL) {
		ctr_heap_free( prg );
		return CtrStdNil;
	}
	ctr_heap_free( prg );
	ctr_cwlk_subprogram++;
	ctr_cwlk_run(parsedCode);
	ctr_cwlk_subprogram--;
	return myself;
}

/**
 * @def
 * [ Program ] arguments
 *
 * @example
 * ✎ write: Program arguments, stop.
 */
ctr_object* ctr_program_num_of_args(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float( (ctr_number) ctr_argc );
}

/**
 * @def
 * [ Program ] end
 *
 * @example
 * ✎ write: ‘1..2..3..’, stop.
 * Program end
 * ✎ write: ‘4..5..6..’, stop.
 */
ctr_object* ctr_program_exit(ctr_object* myself, ctr_argument* argumentList) {
	CtrStdFlow = CtrStdExit;
	return CtrStdNil;
}

/**
 * @def
 * [ Program ] setting: [ String ]
 *
 * @example
 * ☞ x ≔ Program setting: ‘SHELL’.
 * ✎ write: x, stop.
 * Program setting: ‘TEST’ value: ‘123’.
 * ☞ x ≔ Program setting: ‘TEST’.
 * ✎ write: x, stop.
 */
ctr_object* ctr_program_get_env(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* envVarNameObj;
	char*       envVarNameStr;
	char*       envVal;
	envVarNameObj = ctr_internal_cast2string(argumentList->object);
	envVarNameStr = ctr_heap_allocate((envVarNameObj->value.svalue->vlen+1)*sizeof(char));
	strncpy(envVarNameStr, envVarNameObj->value.svalue->value, envVarNameObj->value.svalue->vlen);
	*(envVarNameStr + (envVarNameObj->value.svalue->vlen)) = '\0';
	envVal = getenv(envVarNameStr);
	ctr_heap_free(envVarNameStr );
	if (envVal == NULL) {
		return CtrStdNil;
	}
	return ctr_build_string_from_cstring(envVal);
}

/**
 * @def
 * [ Program ] setting: [ String ] value: [ String ]
 *
 * @example
 * ☞ x ≔ Program setting: ‘SHELL’.
 * ✎ write: x, stop.
 * Program setting: ‘TEST’ value: ‘123’.
 * ☞ x ≔ Program setting: ‘TEST’.
 * ✎ write: x, stop.
 */
ctr_object* ctr_program_set_env(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* envVarNameObj;
	ctr_object* envValObj;
	char*       envVarNameStr;
	char*       envValStr;
	envVarNameObj = ctr_internal_cast2string(argumentList->object);
	envValObj = ctr_internal_cast2string(argumentList->next->object);
	envVarNameStr = ctr_heap_allocate_cstring( envVarNameObj );
	envValStr = ctr_heap_allocate_cstring( envValObj );
	setenv(envVarNameStr, envValStr, 1);
	ctr_heap_free( envValStr );
	ctr_heap_free( envVarNameStr );
	return myself;
}

/**
 * @def
 * [ Program ] ask
 *
 * @example
 * ✎ write: Program ask, stop.
 *
 * @result
 * ~$ hello_
 * hello
 * ~$_
 */
ctr_object* ctr_program_waitforinput(ctr_object* myself, ctr_argument* argumentList) {
	int c;
	ctr_size bytes = 0;
	char* buff;
	ctr_size page = 10;
	ctr_object* userInput;
	buff = ctr_heap_allocate(page * sizeof(char));
	while ((c = getchar()) != '\n') {
		buff[bytes] = c;
		bytes++;
		if (bytes >= page) {
			page *= 2;
			buff = (char*) ctr_heap_reallocate(buff, page * sizeof(char));
			if (buff == NULL) {
				CtrStdFlow = ctr_error( CTR_ERR_OOM, 0 );
				return CtrStdNil;
			}
		}
	}
	userInput = ctr_build_string(buff, bytes);
	ctr_heap_free(buff);
	return userInput;
}

/**
 * @def
 * [ Program ] input.
 *
 * @example
 * ☞ a ≔ Program input.
 * ✎ write: a, stop.
 * 
 * @result
 * ~$ echo 123 | x.ctr
 * 123
 * ~$_
 */
ctr_object* ctr_program_input(ctr_object* myself, ctr_argument* argumentList) {
	ctr_size bytes = 0;
	ctr_size page = 64;
	char buffer[page];
	size_t content_size = 0;
	char *content = ctr_heap_allocate(0);
	clearerr(stdin);
	int reading = 1;
	while(reading) {
		bytes = fread(buffer, sizeof(char), page, stdin);
		content_size += bytes;
		content = ctr_heap_reallocate(content, content_size);
		memcpy(content + (content_size - bytes),buffer,bytes);
		reading = !(bytes != page && feof(stdin));
	}
	/* strip the last newline */
	if ( content_size > 0 && *(content+(content_size-1))=='\n' ) {
		content_size -= 1;
	}
	ctr_object* str = ctr_build_string( content, content_size );
	ctr_heap_free( content );
	return str;
}

/**
 * @def
 * [ Program ] flush.
 *
 * @example
 * ✎ write: ‘Flush stdout buffer.’, stop.
 * Program flush.
 */
ctr_object* ctr_program_flush(ctr_object* myself, ctr_argument* ctr_argumentList) {
	 fflush(stdout);
	 return myself;
}

/**
 * @def
 * [ Program ] error: [ String ]
 *
 * @example
 * Program error: ‘123’.
 * 
 * @result
 * ~$ x.ctr 2>1
 * 123
 * ~$_
 */
ctr_object* ctr_program_err(ctr_object* myself, ctr_argument* argumentList) {
	char* message;
	message = ctr_heap_allocate_cstring(
		ctr_internal_cast2string(
			argumentList->object
		)
	);
	fwrite( message, sizeof(char), strlen(message), stderr);
	fwrite( "\n", sizeof(char), 1, stderr);
	ctr_heap_free( message );
	return myself;
}

/**
 * @def
 * [ Moment ] wait: [ Number ]
 *
 * @example
 * Program wait: 2.
 * 
 * @result
 * ~$ x.ctr
 * .... waits sec ....
 * .... waits sec ....
 * ~$_
 */
ctr_object* ctr_clock_wait(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* arg = ctr_internal_cast2number(argumentList->object);
	int n = (int) arg->value.nvalue;
	sleep(n);
	return myself;
}

/**
 * @def
 * Moment
 * 
 * @example
 * ☞ x ≔ Moment new.
 * ✎ write: x, stop.
 */
ctr_object* ctr_clock_new_set( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_object* clock;
	clock = ctr_clock_new( myself, argumentList );
	ctr_internal_object_add_property( clock, ctr_build_string_from_cstring( CTR_DICT_TIME ), ctr_internal_cast2number(argumentList->object), CTR_CATEGORY_PRIVATE_PROPERTY );
	return clock;
}

/**
 * @internal
 */
ctr_object* ctr_clock_get_time( ctr_object* myself, ctr_argument* argumentList, char part ) {
	struct tm* date;
	time_t timeStamp;
	ctr_object* answer;
	char* zone;
	timeStamp = (time_t) ctr_internal_cast2number(
		ctr_internal_object_find_property( myself, ctr_build_string_from_cstring(CTR_DICT_TIME), CTR_CATEGORY_PRIVATE_PROPERTY )
	)->value.nvalue;
	zone = ctr_heap_allocate_cstring(
		ctr_internal_cast2string(
			ctr_internal_object_find_property( myself, ctr_build_string_from_cstring(CTR_DICT_ZONE), CTR_CATEGORY_PRIVATE_PROPERTY )
		)
	);
	setenv( "TZ", zone, 1 );
	date = localtime( &timeStamp );
	setenv( "TZ", "UTC", 1 );
	switch( part ) {
		case 'Y':
			answer = ctr_build_number_from_float( (ctr_number) date->tm_year + 1900 );
			break;
		case 'm':
			answer = ctr_build_number_from_float( (ctr_number) date->tm_mon + 1 );
			break;
		case 'd':
			answer = ctr_build_number_from_float( (ctr_number) date->tm_mday );
			break;
		case 'H':
			answer = ctr_build_number_from_float( (ctr_number) date->tm_hour );
			break;
		case 'i':
			answer = ctr_build_number_from_float( (ctr_number) date->tm_min );
			break;
		case 's':
			answer = ctr_build_number_from_float( (ctr_number) date->tm_sec );
			break;
	}
	ctr_heap_free( zone );
	return answer;
}

/**
 * @internal
 */
ctr_object* ctr_clock_set_time( ctr_object* myself, ctr_argument* argumentList, char part ) {
	struct tm* date;
	time_t timeStamp;
	ctr_object* key;
	char* zone;
	key = ctr_build_string_from_cstring( CTR_DICT_TIME );
	timeStamp = (time_t) ctr_internal_cast2number(
		ctr_internal_object_find_property( myself, key, 0 )
	)->value.nvalue;
	zone = ctr_heap_allocate_cstring(
		ctr_internal_cast2string(
			ctr_internal_object_find_property( myself, ctr_build_string_from_cstring(CTR_DICT_ZONE), CTR_CATEGORY_PRIVATE_PROPERTY )
		)
	);
	setenv( "TZ", zone, 1 );
	date = localtime( &timeStamp );
	switch( part ) {
		case 'Y':
			date->tm_year = ctr_internal_cast2number(argumentList->object)->value.nvalue - 1900;
			date->tm_mon = 0;
			date->tm_mday = 1;
			date->tm_hour = 0;
			date->tm_min = 0;
			date->tm_sec = 0;
			break;
		case 'm':
			date->tm_mon = ctr_internal_cast2number(argumentList->object)->value.nvalue - 1;
			date->tm_mday = 1;
			date->tm_hour = 0;
			date->tm_min = 0;
			date->tm_sec = 0;
			break;
		case 'd':
			date->tm_mday = ctr_internal_cast2number(argumentList->object)->value.nvalue;
			date->tm_hour = 0;
			date->tm_min = 0;
			date->tm_sec = 0;
			break;
		case 'H':
			date->tm_hour = ctr_internal_cast2number(argumentList->object)->value.nvalue;
			date->tm_min = 0;
			date->tm_sec = 0;
			break;
		case 'i':
			date->tm_min = ctr_internal_cast2number(argumentList->object)->value.nvalue;
			date->tm_sec = 0;
			break;
		case 's':
			date->tm_sec = ctr_internal_cast2number(argumentList->object)->value.nvalue;
			break;
	}
	date->tm_isdst = -1;
	ctr_heap_free( zone );
	ctr_internal_object_set_property( myself, key, ctr_build_number_from_float( (double_t) mktime( date ) ), 0 );
	setenv( "TZ", "UTC", 1 );
	return myself;
}

/**
 * @def
 * [ Moment ] zone: [ String ]
 *
 * @example
 * ☞ t ≔ Moment new.
 * t zone: ‘Europe/Amsterdam’.
 * ✎ write: t, stop.
 * ✎ write: t zone, stop.
 * t zone: ‘US/Hawaii’.
 * ✎ write: t, stop.
 * ✎ write: t zone, stop.
 */
ctr_object* ctr_clock_set_zone( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_internal_object_set_property( myself, ctr_build_string_from_cstring(CTR_DICT_ZONE), ctr_internal_cast2string( argumentList->object ), CTR_CATEGORY_PRIVATE_PROPERTY );
	return myself;
}

/**
 * @def
 * [ Moment ] zone
 *
 * @example
 * ☞ t ≔ Moment new.
 * t zone: ‘Europe/Amsterdam’.
 * ✎ write: t, stop.
 * ✎ write: t zone, stop.
 * t zone: ‘US/Hawaii’.
 * ✎ write: t, stop.
 * ✎ write: t zone, stop.
 */
ctr_object* ctr_clock_get_zone( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_internal_object_find_property( myself, ctr_build_string_from_cstring(CTR_DICT_ZONE), CTR_CATEGORY_PRIVATE_PROPERTY );
}

/**
 * @def
 * [ Moment ] year: [ Number ]
 *
 * @example
 * ☞ x ≔ Moment new.
 * x year: 2020.
 * x month: 8.
 * x day: 12.
 * x hour: 10.
 * x minute: 2.
 * x second: 1.
 * ✎ write: x year.
 * ✎ write: x month.
 * ✎ write: x day.
 * ✎ write: x hour.
 * ✎ write: x minute.
 * ✎ write: x second.
 */
ctr_object* ctr_clock_set_year( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_set_time( myself, argumentList, 'Y' );
}

/**
 * @def
 * [ Moment ] month: [ Number ]
 *
 * @example
 * ☞ x ≔ Moment new.
 * x year: 2020.
 * x month: 8.
 * x day: 12.
 * x hour: 10.
 * x minute: 2.
 * x second: 1.
 * ✎ write: x year.
 * ✎ write: x month.
 * ✎ write: x day.
 * ✎ write: x hour.
 * ✎ write: x minute.
 * ✎ write: x second.
 */
ctr_object* ctr_clock_set_month( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_set_time( myself, argumentList, 'm' );
}

/**
 * @def
 * [ Moment ] day: [ Number ]
 *
 * @example
 * ☞ x ≔ Moment new.
 * x year: 2020.
 * x month: 8.
 * x day: 12.
 * x hour: 10.
 * x minute: 2.
 * x second: 1.
 * ✎ write: x year.
 * ✎ write: x month.
 * ✎ write: x day.
 * ✎ write: x hour.
 * ✎ write: x minute.
 * ✎ write: x second.
 */
ctr_object* ctr_clock_set_day( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_set_time( myself, argumentList, 'd' );
}

/**
 * @def
 * [ Moment ] hour: [ Number ]
 *
 * @example
 * ☞ x ≔ Moment new.
 * x year: 2020.
 * x month: 8.
 * x day: 12.
 * x hour: 10.
 * x minute: 2.
 * x second: 1.
 * ✎ write: x year.
 * ✎ write: x month.
 * ✎ write: x day.
 * ✎ write: x hour.
 * ✎ write: x minute.
 * ✎ write: x second.
 */
ctr_object* ctr_clock_set_hour( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_set_time( myself, argumentList, 'H' );
}

/**
 * @def
 * [ Moment ] minute: [ Number ]
 *
 * @example
 * ☞ x ≔ Moment new.
 * x year: 2020.
 * x month: 8.
 * x day: 12.
 * x hour: 10.
 * x minute: 2.
 * x second: 1.
 * ✎ write: x year.
 * ✎ write: x month.
 * ✎ write: x day.
 * ✎ write: x hour.
 * ✎ write: x minute.
 * ✎ write: x second.
 */
ctr_object* ctr_clock_set_minute( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_set_time( myself, argumentList, 'i' );
}

/**
 * @def
 * [ Moment ] second: [ Number ]
 *
 * @example
 * ☞ x ≔ Moment new.
 * x year: 2020.
 * x month: 8.
 * x day: 12.
 * x hour: 10.
 * x minute: 2.
 * x second: 1.
 * ✎ write: x year.
 * ✎ write: x month.
 * ✎ write: x day.
 * ✎ write: x hour.
 * ✎ write: x minute.
 * ✎ write: x second.
 */
ctr_object* ctr_clock_set_second( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_set_time( myself, argumentList, 's' );
}

/**
 * @def
 * [ Moment ] year
 *
 * @example
 * ☞ x ≔ Moment new.
 * x year: 2020.
 * x month: 8.
 * x day: 12.
 * x hour: 10.
 * x minute: 2.
 * x second: 1.
 * ✎ write: x year.
 * ✎ write: x month.
 * ✎ write: x day.
 * ✎ write: x hour.
 * ✎ write: x minute.
 * ✎ write: x second.
 */
ctr_object* ctr_clock_year( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_get_time( myself, argumentList, 'Y' );
}

/**
 * @def
 * [ Moment ] month
 *
 * @example
 * ☞ x ≔ Moment new.
 * x year: 2020.
 * x month: 8.
 * x day: 12.
 * x hour: 10.
 * x minute: 2.
 * x second: 1.
 * ✎ write: x year.
 * ✎ write: x month.
 * ✎ write: x day.
 * ✎ write: x hour.
 * ✎ write: x minute.
 * ✎ write: x second.
 */
ctr_object* ctr_clock_month( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_get_time( myself, argumentList, 'm' );
}

/**
 * @def
 * [ Moment ] day
 *
 * @example
 * ☞ x ≔ Moment new.
 * x year: 2020.
 * x month: 8.
 * x day: 12.
 * x hour: 10.
 * x minute: 2.
 * x second: 1.
 * ✎ write: x year.
 * ✎ write: x month.
 * ✎ write: x day.
 * ✎ write: x hour.
 * ✎ write: x minute.
 * ✎ write: x second.
 */
ctr_object* ctr_clock_day( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_get_time( myself, argumentList, 'd' );
}

/**
 * @def
 * [ Moment ] hour
 *
 * @example
 * ☞ x ≔ Moment new.
 * x year: 2020.
 * x month: 8.
 * x day: 12.
 * x hour: 10.
 * x minute: 2.
 * x second: 1.
 * ✎ write: x year.
 * ✎ write: x month.
 * ✎ write: x day.
 * ✎ write: x hour.
 * ✎ write: x minute.
 * ✎ write: x second.
 */
ctr_object* ctr_clock_hour( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_get_time( myself, argumentList, 'H' );
}

/**
 * @def
 * [ Moment ] minute
 *
 * @example
 * ☞ x ≔ Moment new.
 * x year: 2020.
 * x month: 8.
 * x day: 12.
 * x hour: 10.
 * x minute: 2.
 * x second: 1.
 * ✎ write: x year.
 * ✎ write: x month.
 * ✎ write: x day.
 * ✎ write: x hour.
 * ✎ write: x minute.
 * ✎ write: x second.
 */
ctr_object* ctr_clock_minute( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_get_time( myself, argumentList, 'i' );
}

/**
 * @def
 * [ Moment ] second
 *
 * @example
 * ☞ x ≔ Moment new.
 * x year: 2020.
 * x month: 8.
 * x day: 12.
 * x hour: 10.
 * x minute: 2.
 * x second: 1.
 * ✎ write: x year.
 * ✎ write: x month.
 * ✎ write: x day.
 * ✎ write: x hour.
 * ✎ write: x minute.
 * ✎ write: x second.
 */
ctr_object* ctr_clock_second( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_get_time( myself, argumentList, 's' );
}

/**
 * @def
 * [ Moment ] day of the year
 *
 * @example
 * ☞ t ≔ Moment new.
 * ✎ write: t, stop.
 * ✎ write: t day of the year, stop.
 */
ctr_object* ctr_clock_yearday( ctr_object* myself, ctr_argument* argumentList ) {
	struct tm* date;
	time_t timeStamp;
	timeStamp = (time_t) ctr_internal_cast2number(
		ctr_internal_object_find_property( myself, ctr_build_string_from_cstring(CTR_DICT_TIME), 0 )
	)->value.nvalue;
	date = localtime( &timeStamp );
	return ctr_build_number_from_float( (double_t) date->tm_yday );
}

/**
 * @def
 * [ Moment ] weekday
 *
 * @example
 * ☞ t ≔ Moment new.
 * ✎ write: t, stop.
 * ✎ write: t weekday, stop.
 */
ctr_object* ctr_clock_weekday( ctr_object* myself, ctr_argument* argumentList ) {
	struct tm* date;
	time_t timeStamp;
	timeStamp = (time_t) ctr_internal_cast2number(
		ctr_internal_object_find_property( myself, ctr_build_string_from_cstring(CTR_DICT_TIME), 0 )
	)->value.nvalue;
	date = localtime( &timeStamp );
	return ctr_build_number_from_float( (double_t) date->tm_wday + 1 );
}

/**
 * @def
 * [ Moment ] time.
 *
 * @example
 * ☞ t ≔ Moment new time.
 * ✎ write: t, stop.
 */
ctr_object* ctr_clock_time( ctr_object* myself, ctr_argument* argumentList ) {
	time_t timeStamp;
	timeStamp = (time_t) ctr_internal_cast2number(
		ctr_internal_object_find_property( myself, ctr_build_string_from_cstring(CTR_DICT_TIME), 0 )
	)->value.nvalue;
	return ctr_build_number_from_float( (double_t) timeStamp );
}

/**
 * @def
 * [ Moment ] copy
 *
 * @example
 * ☞ t ≔ Moment new.
 * ✎ write: t, stop.
 * ☞ t2 ≔ t copy.
 * ✎ write: t2, stop.
 */
ctr_object* ctr_clock_copy( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_object* clock;
	clock = ctr_clock_new( myself, argumentList );
	ctr_internal_object_add_property( clock,
		ctr_build_string_from_cstring( CTR_DICT_TIME ),
		ctr_clock_time( myself, NULL ),
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	ctr_internal_object_add_property( clock,
		ctr_build_string_from_cstring( CTR_DICT_ZONE ),
		ctr_clock_get_zone( myself, NULL ),
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	return clock;
}

/**
 * @def
 * [ Moment ] = [ Moment ]
 *
 * @example
 * ☞ m ≔ Moment new year: 2070, month: 11.
 * ☞ n ≔ m copy.
 * ✎ write: (m = n), stop.
 * ✎ write: (m ≠ n), stop.
 * ✎ write: (m equals: m), stop.
 * ✎ write: (m equals: n), stop.
 * n month: 12.
 * ✎ write: (m ≠ n), stop.
 * ✎ write: (m = n), stop.
 */
ctr_object* ctr_clock_equals( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_object* mytime = ctr_clock_time( myself, NULL );
	ctr_object* myzone = ctr_clock_get_zone( myself, NULL );
	ctr_object* othertime = ctr_clock_time( argumentList->object, NULL );
	ctr_object* otherzone = ctr_internal_cast2string(ctr_clock_get_zone( argumentList->object, NULL ));
	if (
	mytime->value.nvalue == othertime->value.nvalue &&
	myzone != NULL &&
	otherzone != NULL &&
	strncmp(myzone->value.svalue->value,otherzone->value.svalue->value,myzone->value.svalue->vlen)==0
	) {
		return ctr_build_bool(1);
	}
	return ctr_build_bool(0);
}

/**
 * @def
 * [ Moment ] ≠ [ Moment ]
 *
 * @example
 * ☞ m ≔ Moment new year: 2070, month: 11.
 * ☞ n ≔ m copy.
 * ✎ write: (m = n), stop.
 * ✎ write: (m ≠ n), stop.
 * ✎ write: (m equals: m), stop.
 * ✎ write: (m equals: n), stop.
 * n month: 12.
 * ✎ write: (m ≠ n), stop.
 * ✎ write: (m = n), stop.
 */
ctr_object* ctr_clock_neq( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_object* bool = ctr_clock_equals(myself, argumentList);
	bool->value.bvalue = !bool->value.bvalue;
	return bool;
}


/**
 * @def
 * [ Moment ] week
 *
 * @example
 * ☞ t ≔ Moment new.
 * ✎ write: t, stop.
 * ✎ write: t week, stop.
 */
ctr_object* ctr_clock_week( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_object* weekNumber;
	char*  str;
	time_t timeStamp;
	timeStamp = (time_t) ctr_internal_cast2number(
		ctr_internal_object_find_property( myself, ctr_build_string_from_cstring(CTR_DICT_TIME), 0 )
	)->value.nvalue;
	str = ctr_heap_allocate( 4 );
	strftime( str, 3, "%W", localtime( &timeStamp ) );
	weekNumber = ctr_internal_cast2number( ctr_build_string_from_cstring( str ) );
	ctr_heap_free( str );
	return weekNumber;
}

/**
 * @def
 * [ Moment ] string
 *
 * @example
 * Moment on: ‘format:’ do: { ↲ ‘beautiful moment’. }.
 * Moment on: ‘time’ do: { ↲ ‘999’. }.
 * ✎ write: Moment, end.
 * ✎ write: Moment number, end.
 */
ctr_object* ctr_clock_to_string( ctr_object* myself, ctr_argument* argumentList ) {
	char*       zone;
	char*       description;
	ctr_object* answer;
	time_t      timeStamp;
	char*       format;
	format = CTR_STDDATEFRMT;
	zone = ctr_heap_allocate_cstring(
		ctr_internal_cast2string(
			ctr_internal_object_find_property( myself, ctr_build_string_from_cstring(CTR_DICT_ZONE), CTR_CATEGORY_PRIVATE_PROPERTY )
		)
	);
	timeStamp = (time_t) ctr_internal_cast2number(
		ctr_internal_object_find_property( myself, ctr_build_string_from_cstring(CTR_DICT_TIME), 0 )
	)->value.nvalue;
	description = ctr_heap_allocate( 41 );
	setenv( "TZ", zone, 1 );
	strftime( description, 40, format, localtime( &timeStamp ) );
	setenv( "TZ", "UTC", 1 );
	answer = ctr_build_string_from_cstring( description );
	ctr_heap_free( description );
	ctr_heap_free( zone );
	return answer;
}

/**
 * @def
 * [ Moment ] number
 *
 * @example
 * ☞ x ≔ Moment new.
 * ✎ write: x, stop.
 * ✎ write: x number, stop.
 */
ctr_object* ctr_clock_to_number( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_send_message( myself, "time", strlen("time"), argumentList );
}

/**
 * @internal
 */
void ctr_clock_init( ctr_object* clock ) {
	ctr_internal_object_add_property( clock, ctr_build_string_from_cstring( CTR_DICT_TIME ), ctr_build_number_from_float( (double_t) time( NULL ) ), 0 );
	ctr_internal_object_add_property( clock, ctr_build_string_from_cstring( CTR_DICT_ZONE ), ctr_build_string_from_cstring( CTR_STDTIMEZONE ), 0 );
}

/**
 * @internal
 */
ctr_object* ctr_clock_change( ctr_object* myself, ctr_argument* argumentList, uint8_t forward ) {
	ctr_number number;
	ctr_object* numberObject;
	ctr_object* qual;
	char* zone;
	ctr_object* timeObject;
	ctr_size l;
	time_t time;
	char* unit;
	struct tm* date;
	numberObject = ctr_internal_cast2number( argumentList->object );
	number = numberObject->value.nvalue * (forward ? 1 : -1);
	qual = ctr_internal_object_find_property( argumentList->object, ctr_build_string_from_cstring(CTR_DICT_QUALIFICATION), CTR_CATEGORY_PRIVATE_PROPERTY );
	if (qual == NULL) {
		return myself;
	}
	qual = ctr_internal_cast2string( qual );
	unit = qual->value.svalue->value;
	l    = qual->value.svalue->vlen;
	timeObject = ctr_internal_object_find_property( myself, ctr_build_string_from_cstring(CTR_DICT_TIME), CTR_CATEGORY_PRIVATE_PROPERTY );
	if ( timeObject == NULL ) {
		return myself;
	}
	time = (time_t) ctr_internal_cast2number( timeObject )->value.nvalue;
	zone = ctr_heap_allocate_cstring(
		ctr_internal_cast2string(
			ctr_internal_object_find_property( myself, ctr_build_string_from_cstring(CTR_DICT_ZONE), CTR_CATEGORY_PRIVATE_PROPERTY )
		)
	);
	setenv( "TZ", zone, 1 );
	date = localtime( &time );
	if ( strncmp( unit, CTR_DICT_HOUR, l ) == 0 ) {
		date->tm_hour += number;
	} else if ( strncmp( unit, CTR_DICT_MINUTE, l ) == 0 ) {
		date->tm_min += number;
	} else if ( strncmp( unit, CTR_DICT_SECOND, l ) == 0 ) {
		date->tm_sec += number;
	} else if ( strncmp( unit, CTR_DICT_DAY, l ) == 0 ) {
		date->tm_mday += number;
	} else if ( strncmp( unit, CTR_DICT_MONTH, l ) == 0 ) {
		date->tm_mon += number;
	} else if ( strncmp( unit, CTR_DICT_YEAR, l ) == 0 ) {
		date->tm_year += number;
	} else if ( strncmp( unit, CTR_DICT_WEEK, l ) == 0 ) {
		date->tm_mday += number * 7;
	}
	ctr_internal_object_set_property( myself, ctr_build_string_from_cstring(CTR_DICT_TIME), ctr_build_number_from_float( (ctr_number) mktime( date ) ), CTR_CATEGORY_PRIVATE_PROPERTY  );
	setenv( "TZ", "UTC", 1 );
	ctr_heap_free( zone );
	return myself;
}

/**
 * @def
 * [ Moment ] add: [ Number ].
 * 
 * @example
 * ☞ x ≔ Moment new.
 * ✎ write: x, stop.
 * x add: 6 second.
 * ✎ write: x, stop.
 * x add: 5 minute.
 * ✎ write: x, stop.
 * x add: 4 hour.
 * ✎ write: x, stop.
 * x add: 3 day.
 * ✎ write: x, stop.
 * x add: 2 month.
 * ✎ write: x, stop.
 * x add: 1 year.
 * ✎ write: x, stop.
 */
ctr_object* ctr_clock_add( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_change( myself, argumentList, 1 );
}

/**
 * [ Moment ] subtract: [Number].
 *
 * Same as [ Moment ] add: but subtracts the number instead of adding it to
 * the clock‘s time.
 */
ctr_object* ctr_clock_subtract( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_change( myself, argumentList, 0 );
}

/**
 * [ Moment ] new
 *
 * Creates a new clock, by default a clock will be set to
 * the local timezone and the current time.
 */
ctr_object* ctr_clock_new( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_object* clock;
	clock = ctr_internal_create_object( CTR_OBJECT_TYPE_OTOBJECT );
	clock->link = myself;
	ctr_clock_init( clock );
	return clock;
}

/**
 * @def
 * ✎ write: [ String ]
 *
 * @example
 * ✎ write: ‘Hello World’.
 */
ctr_object* ctr_console_write(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* argument1 = argumentList->object;
	ctr_object* strObject = ctr_internal_cast2string(argument1);
	fwrite(strObject->value.svalue->value, sizeof(char), strObject->value.svalue->vlen, stdout);
	return myself;
}

/**
 * @def
 * ✎ stop
 *
 * @example
 * ✎ stop.
 * ✎ stop.
 * ✎ stop.
 */
ctr_object* ctr_console_brk(ctr_object* myself, ctr_argument* argumentList) {
	fwrite("\n", sizeof(char), 1, stdout);
	return myself;
}



/**
 * @internal
 *
 * Shell Object uses a fluid API.
 */
ctr_object* ctr_slurp_respond_to(ctr_object* myself, ctr_argument* argumentList) {
	ctr_argument* newArgumentList;
	ctr_object*   newCommandObj;
	ctr_object*   commandObj;
	ctr_object*   key;
	newCommandObj = argumentList->object;
	key = ctr_build_string_from_cstring( "command" );
	commandObj = ctr_internal_object_find_property( myself, key, CTR_CATEGORY_PRIVATE_PROPERTY );
	newArgumentList = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	if ( commandObj == NULL ) {
		commandObj = ctr_build_empty_string();
		ctr_internal_object_set_property( myself, key, commandObj, CTR_CATEGORY_PRIVATE_PROPERTY );
	} else {
		newArgumentList->object = ctr_internal_object_find_property( myself, ctr_build_string_from_cstring( "glue" ), CTR_CATEGORY_PRIVATE_PROPERTY );
		ctr_string_append( commandObj, newArgumentList );
	}
	newArgumentList->object = newCommandObj;
	ctr_string_append( commandObj, newArgumentList );
	ctr_heap_free( newArgumentList );
	return myself;
}

ctr_object* ctr_slurp_glue_set(ctr_object* myself, char* glue) {
	ctr_internal_object_set_property( myself, ctr_build_string_from_cstring( "glue" ), ctr_build_string_from_cstring(glue), CTR_CATEGORY_PRIVATE_PROPERTY );
	return myself;
}

ctr_object* ctr_path_respond_to(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* pathObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	pathObject->link = CtrStdSlurp;
	ctr_slurp_glue_set(pathObject, CTR_DIRSEP);
	return ctr_slurp_respond_to(pathObject, argumentList);
}

ctr_object* ctr_path_respond_to_and(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* pathObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	pathObject->link = CtrStdSlurp;
	ctr_slurp_glue_set(pathObject, CTR_DIRSEP);
	return ctr_slurp_respond_to_and(pathObject, argumentList);
}

ctr_object* ctr_shellcommand_respond_to(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* shellCommandObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	shellCommandObject->link = CtrStdSlurp;
	ctr_slurp_glue_set(shellCommandObject, " ");
	return ctr_slurp_respond_to(shellCommandObject, argumentList);
}

ctr_object* ctr_shellcommand_respond_to_and(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* shellCommandObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	shellCommandObject->link = CtrStdSlurp;
	ctr_slurp_glue_set(shellCommandObject, " ");
	return ctr_slurp_respond_to_and(shellCommandObject, argumentList);
}

/**
 * @internal
 * 
 * Slurp uses a fluid API
 */
ctr_object* ctr_slurp_respond_to_and(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* str;
	ctr_argument* newArgumentList;
	newArgumentList = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	str = ctr_internal_cast2string( argumentList->object );
	if ( (str->value.svalue->vlen > 0) && *(str->value.svalue->value + (str->value.svalue->vlen - 1)) == ':' ) {
		char* ncstr = ctr_heap_allocate( str->value.svalue->vlen - 1 );
		memcpy( ncstr, str->value.svalue->value, str->value.svalue->vlen - 1 );
		newArgumentList->object = ctr_build_string( ncstr, str->value.svalue->vlen - 1 );
		ctr_slurp_respond_to( myself, newArgumentList );
		ctr_heap_free( ncstr );
	} else {
		newArgumentList->object = argumentList->object;
		ctr_slurp_respond_to( myself, newArgumentList );
	}
	newArgumentList->object = argumentList->next->object;
	ctr_slurp_respond_to( myself, newArgumentList );
	ctr_heap_free( newArgumentList );
	return myself;
}

/**
 * [Slurp]
 *
 * Slurp is an object that takes any message and converts it to a string.
 * The message ‘obtain‘ can be used to acquire the generated string.
 * The Slurp object is a separate object with minimal messages to avoid
 * ‘message collision‘.
 */

/**
 * [Slurp] obtain.
 * 
 * Obtains the string generated using the Slurp object.
 * A Slurp object collects all messages send to it and flushes its buffer while
 * returning the resulting string after an ‘obtain‘ message has been received.
 * 
 * Usage:
 * 
 * Slurp hello world.
 * Pen write: (Slurp obtain).
 * 
 * This will output: ‘hello world‘.
 * Use the Slurp object to integrate verbose shell commands, other programming languages
 * (like SQL) etc into your main program without overusing strings.
 *
 * Note that we can‘t use the = and * unfortunately right now
 * because = is also a method in the main object. While * can be used
 * theoretically, it expects an identifier, and ‘from‘ is not a real
 * identifier, it‘s just another unary message, so instead of using a binary
 * * we simply use a keyword message select: with argument ‘*‘ and then
 * proceed our SQL query with a comma (,) to chain the rest.
 * This is an artifact of the fact that the DSL has to be embedded within
 * the language of Citrine. However even with these restrictions (some of which might be
 * alleviated in future versions) it‘s quite comfortable and readable to interweave
 * an external language in your Citrine script code.
 *
 * Usage:
 *
 * ☞ query ≔ Slurp
 *	select: ‘*‘,
 *	from
 *		users
 *	where
 *		user_id=: 1.
 *
 * #result: select * from users where user_id= 1
 */
ctr_object* ctr_slurp_obtain( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_object* commandObj;
	ctr_object* key;
	key = ctr_build_string_from_cstring( "command" );
	commandObj = ctr_internal_object_find_property( myself, ctr_build_string_from_cstring( "command" ), CTR_CATEGORY_PRIVATE_PROPERTY );
	if ( commandObj == NULL ) {
		commandObj = ctr_build_empty_string();
	}
	ctr_internal_object_delete_property( myself, key, CTR_CATEGORY_PRIVATE_PROPERTY );
	return commandObj;
}

/**
 * [Slurp] toString.
 *
 * Sending the message ‘toString‘ to a slurp object is the same as sending the
 * obtain message. It will cause the Slurp object to answer with the collected
 * string information from previous interactions. If for some reason the
 * obtain message does not return a string, this message will answer with
 * an empty string, otherwise the resulting string from ‘obtain‘ will be
 * returned.
 */
ctr_object* ctr_slurp_to_string( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_object* commandObj = ctr_slurp_obtain(myself, argumentList);
	if (commandObj->info.type != CTR_OBJECT_TYPE_OTSTRING) {
		return ctr_build_empty_string();
	}
	return commandObj;
}
