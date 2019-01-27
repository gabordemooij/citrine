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
			el->info.mark = 1;
			ctr_gc_mark(el);
		}
	}
	item = object->properties->head;
	while(item) {
		k = item->key;
		o = item->value;
		o->info.mark = 1;
		k->info.mark = 1;
		ctr_gc_mark(o);
		item = item->next;
	} 
	item = object->methods->head;
	while(item) {
		o = item->value;
		k = item->key;
		o->info.mark = 1;
		k->info.mark = 1;
		ctr_gc_mark(o);
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
 * [Program] clean memory
 *
 * GarbageCollector, to invoke use:
 *
 * Program clean memory.
 * 
 * In other languages:
 * Dutch: Programma vegen.
 * Ruimt ongebruikt geheugen op.
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
 * [Program] shell: [String]
 *
 * Performs a Shell operation.
 *
 * Usage:
 *
 * ☞ files := Shell ls
 *
 * In other languages:
 * Dutch: Programma opdrachtregel: [Tekst].
 * Geeft een tekst door aan de opdrachtregel van het
 * besturingsysteem. Deze opdracht zal door het systeem
 * worden afgehandeld.
 * 
 * Voorbeeld (voor Linux/UNIX):
 * ☞ bestanden := Opdrachtregel ls.
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
		CtrStdFlow = ctr_build_string_from_cstring( CTR_ERR_EXEC );
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
 * [Program] argument: [Number]
 *
 * Obtains an argument from the CLI invocation.
 * 
 * In other languages:
 * Dutch: Programma argument: [Getal].
 * Geeft het middels opdrachtregel opgegeven argument terug
 * op aangegeven positie.
 */
ctr_object* ctr_program_argument(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* numberObject = ctr_internal_cast2number(argumentList->object);
	int n = (int) numberObject->value.nvalue;
	if (n >= ctr_argc || n < 0) return CtrStdNil;
	return ctr_build_string(ctr_argv[n], strlen(ctr_argv[n]));
}

/**
 * [Program] arguments
 *
 * Returns the number of CLI arguments passed to the script.
 * 
 * In other languages:
 * Dutch: Programma argumenten.
 * Geeft aantal middels opdrachtregel opgegeven argumenten terug.
 */
ctr_object* ctr_program_num_of_args(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float( (ctr_number) ctr_argc );
}

/**
 * [Program] end
 * 
 * Exits program immediately.
 * 
 * In other languages:
 * Dutch: Programma einde.
 * Stopt de uitvoering van het programma.
 */
ctr_object* ctr_program_exit(ctr_object* myself, ctr_argument* argumentList) {
	CtrStdFlow = CtrStdExit;
	return CtrStdNil;
}

/**
 * [Program] setting: [String]
 *
 * Returns the value of an environment variable.
 *
 * Usage:
 *
 * ☞ x := Command setting: 'MY_PATH_VAR'.
 * 
 * In other languages:
 * Dutch: Programma instelling: [Tekst].
 * Geeft de omgevingsvariabele terug bij de opgegeven sleutel.
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
 * [Program] setting: [Key] value: [Value]
 *
 * Sets the value of an environment variable.
 * 
 * In other languages:
 * Dutch: Programma instelling: [Tekst] waarde: [Tekst].
 * Stelt een omgevingsvariabele in met de opgegeven waarde.
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
 * [Program] ask
 *
 * Ask a question on the command-line, resumes program
 * only after pressing the enter key.
 * Only reads up to 100 characters.
 * The example asks the user for his/her name and
 * then displays the input received.
 *
 * Usage:
 *
 * ✎ write: 'What is your name ?'.
 * ☞ x := Program ask.
 * ✎ write: ('Hello you' you: x), end.
 * 
 * In other languages:
 * Dutch: Programma vraag.
 * Pauzeert de uitvoering van het programma in afwachting van
 * gebruikersinvoer en geeft de invoer als Tekst terug na afloop.
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
		if (bytes > page) {
			page *= 2;
			buff = (char*) ctr_heap_reallocate(buff, page * sizeof(char));
			if (buff == NULL) {
				CtrStdFlow = ctr_build_string_from_cstring( CTR_ERR_OOM );
				return CtrStdNil;
			}
		}
	}
	userInput = ctr_build_string(buff, bytes);
	ctr_heap_free(buff);
	return userInput;
}

/**
 * [Program] input.
 *
 * Reads all raw input from STDIN.
 * The input message reads the standard input stream of the application
 * which allows you to deal with pipes for instance. However this
 * mechanism can also be used to read raw POSTs in case of CGI applications.
 * Note that unlike other implementations the input messages also collects
 * null bytes, a null byte \0 in the input stream will NOT cause it to end.
 * Also note that the trailing newline (in case of CLI applications) will
 * be stripped so you don't have to do this manually. This allows for
 * one-liners like the one in the example below.
 *
 * Usage:
 *
 * echo "hello" | ctr test.ctr
 *
 * ☞ x := Program input.
 * ✎ write: x.
 * 
 * In other languages:
 * Dutch: Programma invoer.
 * Leest alle ruwe invoer vanuit STDIN.
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
 * [Program] flush.
 *
 * Flushes the STDOUT output buffer.
 * 
 * In other languages:
 * Dutch: Programma spoelen.
 * Spoelt de STDOUT buffer. Alles wat in de buffer zit zal naar
 * STDOUT worden geschreven en de buffer zal leeggemaakt worden.
 */
ctr_object* ctr_program_flush(ctr_object* myself, ctr_argument* ctr_argumentList) {
	 fflush(stdout);
	 return myself;
}

/**
 * [Program] error: [String]
 *
 * Logs the specified message string using syslog using log level LOG_ERR.
 * Use this to log errors.
 * 
 * In other languages:
 * Dutch: Programma fout: [Tekst].
 * Schrijft een fout weg in het logboek van het programma.
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
 * [Clock] wait: [Number]
 *
 * Suspends program execution for the specified number of seconds.
 * This can be used for instance, together with a whileFalse loop as the
 * following example illustrates. The following example demonstrates the
 * use of the Clock object and the wait message to wait until an
 * exclusive lock on the specified file has been acquired.
 *
 * Usage:
 *
 * { ↲ file lock. }
 * false: { ⏰ wait: 1. }.
 * 
 * In other languages:
 * Dutch: Moment wacht: [Getal].
 * Wacht het aangegeven aantal seconde alvorens de uitvoering van
 * het programma te hervatten.
 */
ctr_object* ctr_clock_wait(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* arg = ctr_internal_cast2number(argumentList->object);
	int n = (int) arg->value.nvalue;
	sleep(n);
	return myself;
}

/**
 * [Clock] new: [Number]
 *
 * Creates a new clock instance from a UNIX time stamp.
 * 
 * In other languages:
 * Dutch: Moment nieuw: [Getal].
 * Maakt een nieuw tijdsobject aan en stelt de tijd in
 * aan de hand van het UNIX time stamp getal dat wordt
 * meegegeven.
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
			break;
		case 'm':
			date->tm_mon = ctr_internal_cast2number(argumentList->object)->value.nvalue - 1;
			break;
		case 'd':
			date->tm_mday = ctr_internal_cast2number(argumentList->object)->value.nvalue;
			break;
		case 'H':
			date->tm_hour = ctr_internal_cast2number(argumentList->object)->value.nvalue;
			break;
		case 'i':
			date->tm_min = ctr_internal_cast2number(argumentList->object)->value.nvalue;
			break;
		case 's':
			date->tm_sec = ctr_internal_cast2number(argumentList->object)->value.nvalue;
			break;
	}
	ctr_heap_free( zone );
	ctr_internal_object_set_property( myself, key, ctr_build_number_from_float( (double_t) mktime( date ) ), 0 );
	setenv( "TZ", "UTC", 1 );
	return myself;
}

/**
 * [Clock] like: [Clock]
 *
 * Syncs a clock. Copies the time AND zone from the other clock.
 *
 * Usage:
 * 
 * ☞ clock := Clock new: timestamp.
 * ☞ copied := Clock new like: clock.
 * 
 * In other languages:
 * Dutch: Moment zoals: [Moment].
 * Synchroniseert twee momenten. Stelt het eerste moment gelijk aan
 * het tweede.
 */
ctr_object* ctr_clock_like( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_object* otherClock;
	ctr_object* time;
	ctr_object* zone;
	otherClock = argumentList->object;
	time = ctr_internal_object_find_property( otherClock, ctr_build_string_from_cstring( CTR_DICT_TIME ), CTR_CATEGORY_PRIVATE_PROPERTY );
	if ( time == NULL ) {
		time = ctr_build_number_from_float( 0 );
	} else {
		time = ctr_internal_cast2number( time );
	}
	zone = ctr_internal_object_find_property( otherClock, ctr_build_string_from_cstring( CTR_DICT_ZONE ), CTR_CATEGORY_PRIVATE_PROPERTY );
	if ( zone == NULL ) {
		zone = ctr_build_string_from_cstring( "UTC" );
	} else {
		zone = ctr_internal_cast2string( zone );
	}
	ctr_internal_object_set_property( myself, ctr_build_string_from_cstring( CTR_DICT_ZONE ), zone, CTR_CATEGORY_PRIVATE_PROPERTY );
	ctr_internal_object_set_property( myself, ctr_build_string_from_cstring( CTR_DICT_TIME ), time, CTR_CATEGORY_PRIVATE_PROPERTY );
	return myself;
}

/**
 * [Clock] zone: [String]
 *
 * Sets the time zone of the clock.
 * 
 * In other languages:
 * Dutch: [Moment] zone: [Tekst].
 * Stelt de tijdzone van het moment in.
 */
ctr_object* ctr_clock_set_zone( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_internal_object_set_property( myself, ctr_build_string_from_cstring(CTR_DICT_ZONE), ctr_internal_cast2string( argumentList->object ), CTR_CATEGORY_PRIVATE_PROPERTY );
	return myself;
}

/**
 * [Clock] zone
 *
 * Returns time zone of the clock.
 * 
 * In other languages:
 * Dutch: [Moment] zone: [Tekst].
 * Geeft de tijdzone van het moment terug.
 */
ctr_object* ctr_clock_get_zone( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_internal_object_find_property( myself, ctr_build_string_from_cstring(CTR_DICT_ZONE), CTR_CATEGORY_PRIVATE_PROPERTY );
}

/**
 * [Clock] year: [Number]
 *
 * Sets the year of the clock.
 * 
 * In other languages:
 * Dutch: [Moment] jaar: [Getal].
 * Stelt het jaar van een moment in.
 */
ctr_object* ctr_clock_set_year( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_set_time( myself, argumentList, 'Y' );
}

/**
 * [Clock] month: [Number]
 *
 * Sets the month of the clock.
 * 
 * In other languages:
 * Dutch: [Moment] maand: [Getal].
 * Stelt de maand van een moment in.
 */
ctr_object* ctr_clock_set_month( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_set_time( myself, argumentList, 'm' );
}

/**
 * [Clock] day: [Number]
 *
 * Sets the day of the clock.
 * 
 * In other languages:
 * Dutch: [Moment] dag: [Getal].
 * Stelt de dag van het moment in.
 */
ctr_object* ctr_clock_set_day( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_set_time( myself, argumentList, 'd' );
}

/**
 * [Clock] hour: [Number]
 *
 * Sets the hour of the clock.
 * 
 * In other languages:
 * Dutch: [Moment] uur: [Getal].
 * Stelt het korte wijzer van een moment in.
 */
ctr_object* ctr_clock_set_hour( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_set_time( myself, argumentList, 'H' );
}

/**
 * [Clock] minute: [Number]
 *
 * Sets the minute of the clock.
 * 
 * In other languages:
 * Dutch: [Moment] minuut: [Getal].
 * Stelt de lange wijzer van een moment in.
 */
ctr_object* ctr_clock_set_minute( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_set_time( myself, argumentList, 'i' );
}

/**
 * [Clock] second: [Number]
 *
 * Sets the second of the clock.
 * 
 * In other languages:
 * Dutch: [Moment] seconde: [Getal].
 * Stelt de secondewijzer van het moment.
 */
ctr_object* ctr_clock_set_second( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_set_time( myself, argumentList, 's' );
}

/**
 * [Clock] year
 *
 * Returns year of the clock.
 * 
 * In other languages:
 * Dutch: [Moment] jaar.
 * Geeft het jaar van het moment terug.
 */
ctr_object* ctr_clock_year( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_get_time( myself, argumentList, 'Y' );
}

/**
 * [Clock] month
 *
 * Returns month of the clock.
 * 
 * In other languages:
 * Dutch: [Moment] maand.
 * Geeft de maand van het moment terug.
 */
ctr_object* ctr_clock_month( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_get_time( myself, argumentList, 'm' );
}

/**
 * [Clock] day
 *
 * Returns day of the clock.
 * 
 * In other languages:
 * Dutch: [Moment] dag.
 * Geeft de dag van het moment terug.
 */
ctr_object* ctr_clock_day( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_get_time( myself, argumentList, 'd' );
}

/**
 * [Clock] hour
 *
 * Returns hour of the clock.
 * 
 * In other languages:
 * Dutch: [Moment] uur.
 * Geeft het uur van het moment terug.
 */
ctr_object* ctr_clock_hour( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_get_time( myself, argumentList, 'H' );
}

/**
 * [Clock] minute
 *
 * Returns minute of the clock.
 * 
 * In other languages:
 * Dutch: [Moment] minuut.
 * Geeft de minuut van het moment terug.
 */
ctr_object* ctr_clock_minute( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_get_time( myself, argumentList, 'i' );
}

/**
 * [Clock] second
 *
 * Returns second of the clock.
 * 
 * In other languages:
 * Dutch: [Moment] seconde.
 * Geeft de seconde van het moment terug.
 */
ctr_object* ctr_clock_second( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_get_time( myself, argumentList, 's' );
}

/**
 * [Clock] day of the year
 *
 * Returns day number of the year.
 * 
 * In other languages:
 * Dutch: [Moment] jaardag.
 * Geeft het dag van het jaar terug waarop het moment
 * plaatsvindt.
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
 * [Clock] weekday
 *
 * Returns the week day number of the clock.
 * 
 * In other languages:
 * Dutch: [Moment] weekdag.
 * Geeft het dag van de week terug waarop het moment
 * plaatsvindt.
 */
ctr_object* ctr_clock_weekday( ctr_object* myself, ctr_argument* argumentList ) {
	struct tm* date;
	time_t timeStamp;
	timeStamp = (time_t) ctr_internal_cast2number(
		ctr_internal_object_find_property( myself, ctr_build_string_from_cstring(CTR_DICT_TIME), 0 )
	)->value.nvalue;
	date = localtime( &timeStamp );
	return ctr_build_number_from_float( (double_t) date->tm_wday );
}

/**
 * [Clock] time.
 *
 * Returns the UNIX time stamp representation of the time.
 * Note: this is the time OF CREATION OF THE OBJECT. To get the actual time use:
 *
 * Usage:
 *
 * ☞ time := Clock new time.
 * 
 * In other languages:
 * Dutch: [Moment] tijd.
 * Geeft de UNIX tijd van het moment terug.
 * Let op! Dit is de tijd waarop het moment is gemaakt.
 */
ctr_object* ctr_clock_time( ctr_object* myself, ctr_argument* argumentList ) {
	time_t timeStamp;
	timeStamp = (time_t) ctr_internal_cast2number(
		ctr_internal_object_find_property( myself, ctr_build_string_from_cstring(CTR_DICT_TIME), 0 )
	)->value.nvalue;
	return ctr_build_number_from_float( (double_t) timeStamp );
}

/**
 * [Clock] week
 *
 * Returns the week number of the clock.
 * 
 * In other languages:
 * Dutch: [Moment] week.
 * Geeft de week van het jaar terug waarop het moment
 * plaatsvindt.
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
 * [Clock] format: [String]
 *
 * Returns a string describing the date and time represented by the clock object
 * according to the specified format. See strftime for format syntax details.
 * 
 * In other languages:
 * Dutch: [Moment] formaat: [Tekst].
 * Geeft een beschrijving terug van het moment als tekst volgens het ingesteld
 * formaat. Het formaat dient opgegeven te zijn volgens de strftime methode.
 */
ctr_object* ctr_clock_format( ctr_object* myself, ctr_argument* argumentList ) {
	char*       zone;
	char*       description;
	ctr_object* answer;
	time_t      timeStamp;
	char*       format;
	format = ctr_heap_allocate_cstring( ctr_internal_cast2string( argumentList->object ) );
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
	ctr_heap_free( format );
	ctr_heap_free( zone );
	return answer;
}

/**
 * [Clock] string
 *
 * Returns a string describing the date and time
 * represented by the clock object. On receiving this message, the Clock
 * instance will send the message 'format:' to itself and the argument:
 * '%Y-%m-%d %H:%M:%S'. It will return the answer as a string. Note that you
 * can override this behaviour by adding your own 'format:' implementation.
 *
 * Usage:
 *
 * ⏰ on: 'format:' do: { ↲ 'beautiful moment'. }.
 * ⏰ on: 'time' do: { ↲ '999'. }.
 *
 * ✎ write: ⏰, end.
 * ✎ write: ⏰ number, end.
 * 
 * In other languages:
 * Dutch: [Moment] tekst.
 * Geeft een standaard tekstrepresentatie van het moment terug.
 */
ctr_object* ctr_clock_to_string( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_argument* newArgumentList;
	ctr_object*   answer;
	newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	newArgumentList->object = ctr_build_string_from_cstring( "%Y-%m-%d %H:%M:%S" );
	answer = ctr_send_message( myself, CTR_DICT_FORMAT, strlen(CTR_DICT_FORMAT), newArgumentList );
	ctr_heap_free( newArgumentList );
	return answer;
}

/**
 * [Clock] number
 *
 * Returns a timestamp describing the date and time
 * represented by the clock object. On receiving this message, the Clock
 * instance will send the message 'time' to itself
 * and return the answer as a number. Note that you
 * can override this behaviour by adding your own 'time' implementation.
 *
 * Usage:
 *
 * ⏰ on: 'format:' do: { ↲ 'beautiful moment'. }.
 * ⏰ on: 'time' do: { ↲ '999'. }.
 *
 * ✎ write: ⏰, brk.
 * ✎ write: ⏰ number, brk.
 * 
 * In other languages:
 * Dutch: [Moment] getal.
 * Alias voor [Moment] tijd.
 */
ctr_object* ctr_clock_to_number( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_send_message( myself, "time", strlen("time"), argumentList );
}

/**
 * @internal
 */
void ctr_clock_init( ctr_object* clock ) {
	ctr_internal_object_add_property( clock, ctr_build_string_from_cstring( CTR_DICT_TIME ), ctr_build_number_from_float( (double_t) time( NULL ) ), 0 );
	ctr_internal_object_add_property( clock, ctr_build_string_from_cstring( CTR_DICT_ZONE ), ctr_build_string_from_cstring( "UTC" ), 0 );
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
	if ( strncmp( unit, CTR_DICT_HOURS, l ) == 0 || strncmp( unit, CTR_DICT_HOUR, l ) == 0  ) {
		date->tm_hour += number;
	} else if ( strncmp( unit, CTR_DICT_MINUTES, l ) == 0 || strncmp( unit, CTR_DICT_MINUTE, l ) == 0  ) {
		date->tm_min += number;
	} else if ( strncmp( unit, CTR_DICT_SECONDS, l ) == 0 || strncmp( unit, CTR_DICT_SECOND, l ) == 0 ) {
		date->tm_sec += number;
	} else if ( strncmp( unit, CTR_DICT_DAYS, l ) == 0 || strncmp( unit, CTR_DICT_DAY, l ) == 0 ) {
		date->tm_mday += number;
	} else if ( strncmp( unit, CTR_DICT_MONTHS, l ) == 0 || strncmp( unit, CTR_DICT_MONTH, l ) == 0 ) {
		date->tm_mon += number;
	} else if ( strncmp( unit, CTR_DICT_YEARS, l ) == 0 || strncmp( unit, CTR_DICT_YEAR, l ) == 0 ) {
		date->tm_year += number;
	} else if ( strncmp( unit, CTR_DICT_WEEKS, l ) == 0 || strncmp( unit, CTR_DICT_WEEK, l ) == 0 ) {
		date->tm_mday += number * 7;
	}
	ctr_internal_object_set_property( myself, ctr_build_string_from_cstring(CTR_DICT_TIME), ctr_build_number_from_float( (ctr_number) mktime( date ) ), CTR_CATEGORY_PRIVATE_PROPERTY  );
	setenv( "TZ", "UTC", 1 );
	ctr_heap_free( zone );
	return myself;
}

/**
 * [Clock] add: [Number].
 *
 * Adds the number to the clock, updating its time accordingly.
 * Note that this is typically used with a qualifier.
 * If the qualifier is 'hours' the number is treated as hours and
 * the specified number of hours will be added to the time.
 *
 * The Clock object understands the following qualifiers
 * if the selected language is English:
 *
 * sec, second, seconds,
 * min, minute, minutes,
 * hrs, hour, hours,
 * day, days,
 * week, weeks,
 * month, months,
 * year, years
 *
 * Note that it does not matter which form you use, 2 hour means
 * the same as 2 hours (plural).
 *
 * Usage:
 *
 * clock add: 3 minutes.
 * clock add: 1 hour.
 * clock add: 2 second.
 * 
 * In other languages:
 * Dutch: [Moment] optellen: [Getal].
 * Verschuift het moment in de tijd op basis van het getal.
 * Het getal kan gekwalificeerd worden als minuten, uren of seconden.
 */
ctr_object* ctr_clock_add( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_change( myself, argumentList, 1 );
}

/**
 * [Clock] subtract: [Number].
 *
 * Same as '[Clock] add:' but subtracts the number instead of adding it to
 * the clock's time.
 * 
 * In other languages:
 * Dutch: [Moment] aftrekken: [Getal].
 * Verschuift het moment in de tijd op basis van het getal.
 * Het getal kan gekwalificeerd worden als minuten, uren of seconden.
 */
ctr_object* ctr_clock_subtract( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_change( myself, argumentList, 0 );
}

/**
 * [Clock] new
 *
 * Creates a new clock, by default a clock will be set to
 * the UTC timezone having the current time.
 * 
 * In other languages:
 * Dutch: [Moment] nieuw.
 * Maakt een nieuw moment aan voor het huidige tijdstip.
 */
ctr_object* ctr_clock_new( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_object* clock;
	clock = ctr_internal_create_object( CTR_OBJECT_TYPE_OTOBJECT );
	clock->link = myself;
	ctr_clock_init( clock );
	return clock;
}

/**
 * ✎ write: [String]
 *
 * Writes string to console.
 * 
 * In other languages:
 * Dutch: ✎ schrijf: [Tekst].
 * Schrijft de opgegeven tekst naar STDOUT/console.
 */
ctr_object* ctr_console_write(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* argument1 = argumentList->object;
	ctr_object* strObject = ctr_internal_cast2string(argument1);
	fwrite(strObject->value.svalue->value, sizeof(char), strObject->value.svalue->vlen, stdout);
	return myself;
}

/**
 * ✎ stop
 *
 * Outputs a newline character.
 * 
 * In other languages:
 * Dutch: ✎ stop.
 * Sluit een regel af met een einde-regel-symbool.
 */
ctr_object* ctr_console_brk(ctr_object* myself, ctr_argument* argumentList) {
	fwrite("\n", sizeof(char), 1, stdout);
	return myself;
}
