#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#ifdef forLinux
#include <bsd/stdlib.h>
#endif

#include "citrine.h"
#include "siphash.h"

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

/**
 * Broom
 *
 * GarbageCollector, to invoke use:
 *
 * [Broom] sweep.
 */
ctr_object* ctr_gc_collect (ctr_object* myself, ctr_argument* argumentList) {
	ctr_gc_internal_collect(); /* calls internal because automatic GC has to use this function as well and requires low overhead. */
	return myself;
}

/**
 * [Broom] dust
 *
 * Returns the number of objects collected.
 */
ctr_object* ctr_gc_dust(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float((ctr_number) ctr_gc_dust_counter);
}

/**
 * [Broom] objectCount
 *
 * Returns the total number of objects considered in the latest collect
 * cycle.
 */
ctr_object* ctr_gc_object_count(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float((ctr_number) ctr_gc_object_counter);
}

/**
 * [Broom] keptCount
 *
 * Returns the total number of objects that have been marked during the
 * latest cycle and have therefore been allowed to stay in memory.
 */
ctr_object* ctr_gc_kept_count(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float((ctr_number) ctr_gc_kept_counter);
}

/**
 * [Broom] keptAlloc
 *
 * Returns the amount of allocated memory.
 */
ctr_object* ctr_gc_kept_alloc(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float((ctr_number) ctr_gc_alloc);
}

/**
 * [Broom] stickyCount
 *
 * Returns the total number of objects that have a sticky flag.
 * These objects will never be removed.
 */
ctr_object* ctr_gc_sticky_count(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float((ctr_number) ctr_gc_sticky_counter);
}

/**
 * [Broom] memoryLimit
 *
 * Sets the memory limit, if this limit gets exceeded the program will produce
 * an out-of-memory error.
 */
ctr_object* ctr_gc_setmemlimit(ctr_object* myself, ctr_argument* argumentList) {
	ctr_gc_memlimit = (uint64_t) ctr_internal_cast2number( argumentList->object )->value.nvalue;
	return myself;
}

/**
 * [Broom] mode: [Number]
 *
 * Selects mode of operation for GC.
 *
 * Available Modes:
 * 0 - No Garbage Collection
 * 1 - Activate Garbage Collector
 * 4 - Activate Garbage Collector for every single step (testing only)
 */
ctr_object* ctr_gc_setmode(ctr_object* myself, ctr_argument* argumentList) {
	ctr_gc_mode = (int) ctr_internal_cast2number( argumentList->object )->value.nvalue;
	return myself;
}

/**
 * [Shell] call: [String]
 *
 * Performs a Shell operation. The Shell object uses a fluid API, so you can
 * mix shell code with programming logic. For instance to list the contents
 * of a directory use:
 *
 * Shell ls
 *
 * This will output the contents of the current working directly, you
 * can also pass keyword messages like so:
 *
 * Shell echo: 'Hello from the Shell!'.
 *
 * The example above will output the specified message to the console.
 * Every message you send will be turned into a string and dispatched to
 * the 'call:' message.
 */
ctr_object* ctr_shell_call(ctr_object* myself, ctr_argument* argumentList) {
	ctr_check_permission( CTR_SECPRO_NO_SHELL );
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
		CtrStdFlow = ctr_build_string_from_cstring( "Unable to execute command." );
	}
	outputString = ctr_build_empty_string();
	while ( fgets( outputBuffer, 512, stream ) ) {
		appendString = ctr_build_string_from_cstring( outputBuffer );
		newArgumentList->object = appendString;
		ctr_string_append( outputString, newArgumentList );
	}
	ctr_heap_free( outputBuffer );
	ctr_heap_free( newArgumentList );
	ctr_heap_free( comString );
	return outputString;
}

/**
 * @internal
 *
 * Shell Object uses a fluid API.
 */
ctr_object* ctr_shell_respond_to_and(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object*   commandObj;
	ctr_object*   prefix;
	ctr_object*   suffix;
	ctr_argument* newArgumentList;
	ctr_object*   result;
	char* command;
	int len;
	prefix = ctr_internal_cast2string(argumentList->object);
	suffix = ctr_internal_cast2string(argumentList->next->object);
	len = prefix->value.svalue->vlen + suffix->value.svalue->vlen;
	if (len == 0) return myself;
	command = (char*) ctr_heap_allocate( ( sizeof( char ) * len ) ); /* actually we need +1 for the space between commands, but we dont because we remove the colon : !*/
	strncpy(command, prefix->value.svalue->value, prefix->value.svalue->vlen - 1); /* remove colon, gives room for space */
	strncpy(command + (prefix->value.svalue->vlen - 1), " ", 1); /* space to separate commands */
	strncpy(command + (prefix->value.svalue->vlen), suffix->value.svalue->value, suffix->value.svalue->vlen);
	commandObj = ctr_build_string(command, len);
	newArgumentList = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	newArgumentList->object = commandObj;
	result = ctr_shell_call(myself, newArgumentList);
	ctr_heap_free( newArgumentList );
	ctr_heap_free( command );
	return result;
}

/**
 * @internal
 *
 * Shell Object uses a fluid API.
 */
ctr_object* ctr_shell_respond_to(ctr_object* myself, ctr_argument* argumentList) {
	ctr_shell_call(myself, argumentList);
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
		newArgumentList->object = ctr_build_string_from_cstring( " " );
		ctr_string_append( commandObj, newArgumentList );
	}
	newArgumentList->object = newCommandObj;
	ctr_string_append( commandObj, newArgumentList );
	ctr_heap_free( newArgumentList );
	return myself;
}

ctr_object* ctr_slurp_respond_to_and(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* str;
	ctr_argument* newArgumentList;
	newArgumentList = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	str = ctr_internal_cast2string( argumentList->object );
	char* ch =  str->value.svalue->value + (str->value.svalue->vlen - 1);
	
	if ( *ch == ':' ) {
		char* ncstr = ctr_heap_allocate( str->value.svalue->vlen - 1 );
		memcpy( ncstr, str->value.svalue->value, str->value.svalue->vlen -1 );
		newArgumentList->object = ctr_build_string_from_cstring( ncstr );
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

ctr_object* ctr_shell_obtain( ctr_object* myself, ctr_argument* argumentList ) {

	FILE* stream;
	char* outputBuffer;
	ctr_argument* newArgumentList;
	
	

	ctr_object* commandObj;
	ctr_object* appendString;
	ctr_object* outputString;
	
	outputBuffer = ctr_heap_allocate( 512 );
	
	/*commandObj = ctr_internal_object_find_property( myself, ctr_build_string_from_cstring( "command" ), CTR_CATEGORY_PRIVATE_PROPERTY );*/
	
	
	
	
	/*if ( commandObj == NULL ) {
		commandObj = ctr_build_empty_string();
	}*/
	
	newArgumentList = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	
	commandObj = ctr_slurp_obtain( myself, newArgumentList );
	
	
	char* commandString = ctr_heap_allocate_cstring( commandObj );
	

	if ( !( stream = popen( commandString, "r" ) ) ) {
		CtrStdFlow = ctr_build_string_from_cstring( "Unable to execute command." );
	}
	
	outputString = ctr_build_empty_string();
	
	
	while ( fgets( outputBuffer, 512, stream ) ) {
		appendString = ctr_build_string_from_cstring( outputBuffer );
		newArgumentList->object = appendString;
		ctr_string_append( outputString, newArgumentList );
	}
	
	ctr_internal_object_set_property( myself, ctr_build_string_from_cstring( "command" ), ctr_build_empty_string(), CTR_CATEGORY_PRIVATE_PROPERTY );
	
	ctr_heap_free( outputBuffer );
	ctr_heap_free( commandString );
	ctr_heap_free( newArgumentList );
	return outputString;
}



/**
 * [Program] argument: [Number]
 *
 * Obtains an argument from the CLI invocation.
 */
ctr_object* ctr_command_argument(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* numberObject = ctr_internal_cast2number(argumentList->object);
	int n = (int) numberObject->value.nvalue;
	if (n >= ctr_argc || n < 0) return CtrStdNil;
	return ctr_build_string(ctr_argv[n], strlen(ctr_argv[n]));
}

/**
 * [Program] argCount
 *
 * Returns the number of CLI arguments passed to the script.
 */
ctr_object* ctr_command_num_of_args(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float( (ctr_number) ctr_argc );
}

/**
 * [Program] exit
 * 
 * Exits program immediately.
 */
ctr_object* ctr_command_exit(ctr_object* myself, ctr_argument* argumentList) {
	CtrStdFlow = CtrStdExit;
	return CtrStdNil;
}

/**
 * [Program] env: [String]
 *
 * Returns the value of an environment variable.
 *
 * Usage:
 *
 * x := Command env: 'MY_PATH_VAR'.
 */
ctr_object* ctr_command_get_env(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* envVarNameObj;
	char*       envVarNameStr;
	char*       envVal;
	ctr_check_permission( CTR_SECPRO_NO_FILE_READ );
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
 * [Program] env: [Key] val: [Value]
 *
 * Sets the value of an environment variable.
 */
ctr_object* ctr_command_set_env(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* envVarNameObj;
	ctr_object* envValObj;
	char*       envVarNameStr;
	char*       envValStr;
	ctr_check_permission( CTR_SECPRO_NO_FILE_WRITE );
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
 * [Program] waitForInput
 *
 * Ask a question on the command-line, resumes program
 * only after pressing the enter key.
 * Only reads up to 100 characters.
 *
 * Usage:
 *
 * Pen write: 'What is your name ?'.
 * x := Command waitForInput.
 * Pen write: 'Hello ' + x + ' !', brk.
 *
 * The example above asks the user for his/her name and
 * then displays the input received.
 */
ctr_object* ctr_command_waitforinput(ctr_object* myself, ctr_argument* argumentList) {
	ctr_check_permission( CTR_SECPRO_COUNTDOWN );
	int c;
	ctr_size bytes = 0;
	char* buff;
	ctr_size page = 10;
	buff = ctr_heap_allocate(page * sizeof(char));
	while ((c = getchar()) != '\n') {
		buff[bytes] = c;
		bytes++;
		if (bytes > page) {
			page *= 2;
			buff = (char*) realloc(buff, page * sizeof(char));
			if (buff == NULL) {
				CtrStdFlow = ctr_build_string_from_cstring("Out of memory");
				return CtrStdNil;
			}
		}
	}
	return ctr_build_string(buff, bytes);
}

/**
 * [Program] input.
 *
 * Reads all raw input from STDIN.
 *
 * Usage (for instance to read raw CGI post):
 *
 * post := Program input.
 */
ctr_object* ctr_command_input(ctr_object* myself, ctr_argument* argumentList) {
	ctr_size page = 64;
	char buffer[page];
	size_t content_size = 1;
	char *content = ctr_heap_allocate(sizeof(char) * page);
	while(fgets(buffer, page, stdin)) {
		content_size += strlen(buffer);
		content = ctr_heap_reallocate(content, content_size);
		strcat(content, buffer);
	}
	ctr_object* str = ctr_build_string_from_cstring( content );
	ctr_heap_free( content );
	return str;
}

/**
 * @internal
 *
 * Checks whether the user is allowed to perform this kind of operation.
 */
void ctr_check_permission( uint8_t operationID ) {
	char* reason;
	if ( ( ctr_command_security_profile & operationID ) ) {
		reason = "This program is not allowed to perform this operation.";
		if ( operationID == CTR_SECPRO_NO_SHELL ) {
			reason = "This program is not allowed to execute shell commands.";
		}
		if ( operationID == CTR_SECPRO_NO_FILE_WRITE ) {
			reason = "This program is not allowed to modify or delete any files or folders.";
		}
		if ( operationID == CTR_SECPRO_NO_FILE_READ ) {
			reason = "This program is not allowed to perform any file operations.";
		}
		if ( operationID == CTR_SECPRO_NO_INCLUDE ) {
			reason = "This program is not allowed to include any other files for code execution.";
		}
		printf( "%s\n", reason );
		exit(1);
	}
}

/**
 * [Program] forbidShell
 *
 * This method is part of the security profiles feature of Citrine.
 * This will forbid the program to execute any shell operations. All
 * external libraries and plugins are assumed to respect this setting as well.
 *
 * Usage:
 *
 * Program forbidShell.
 */
ctr_object* ctr_command_forbid_shell( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_command_security_profile |= CTR_SECPRO_NO_SHELL;
	return myself;
}

/**
 * [Program] forbidFileWrite
 *
 * This method is part of the security profiles feature of Citrine.
 * This will forbid the program to modify, create or delete any files. All
 * external libraries and plugins are assumed to respect this setting as well.
 *
 * Usage:
 *
 * Program forbidFileWrite.
 */
ctr_object* ctr_command_forbid_file_write( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_command_security_profile |= CTR_SECPRO_NO_FILE_WRITE;
	return myself;
}

/**
 * [Program] forbidFileRead
 *
 * This method is part of the security profiles feature of Citrine.
 * This will forbid the program to read any files. In fact this will prevent you from
 * creating the file object at all.
 * This will also prevent you from reading environment variables.
 * All external libraries and plugins are assumed to respect this setting as well.
 * Forbidding a program to read files also has the effect to forbid including other
 * source files.
 *
 * Usage:
 *
 * Program forbidFileRead.
 */
ctr_object* ctr_command_forbid_file_read( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_command_security_profile |= CTR_SECPRO_NO_FILE_READ;
	return myself;
}

/**
 * [Program] forbidInclude
 *
 * This method is part of the security profiles feature of Citrine.
 * This will forbid the program to include any other files. All
 * external libraries and plugins are assumed to respect this setting as well.
 *
 * Usage:
 *
 * Program forbidInclude.
 */
ctr_object* ctr_command_forbid_include( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_command_security_profile |= CTR_SECPRO_NO_INCLUDE;
	return myself;
}

/**
 * [Program] remainingMessages: [Number]
 *
 * This method is part of the security profiles feature of Citrine.
 * This will initiate a countdown for the program, you can specify the maximum quota of
 * messages the program may process, once this quota has been exhausted the program will
 * be killed entirely (no exception).
 *
 * Usage:
 *
 * Program remainingMessages: 100.
 */
ctr_object* ctr_command_countdown( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_command_security_profile |= CTR_SECPRO_COUNTDOWN;
	ctr_command_maxtick = (uint64_t) ctr_internal_cast2number( argumentList->object )->value.nvalue;
	return myself;
}

/**
 * [Program] flush.
 *
 * Flushes the STDOUT output buffer.
 */
ctr_object* ctr_command_flush(ctr_object* myself, ctr_argument* ctr_argumentList) {
	 fflush(stdout);
	 return myself;
}

/**
 * [Dice] rollWithSides: [Number]
 *
 * Rolls the dice, generates a pseudo random number.
 */
ctr_object* ctr_dice_sides(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* arg = ctr_internal_cast2number(argumentList->object);
	return ctr_build_number_from_float( (ctr_number) 1 + arc4random_uniform( (uint32_t) ( ceil( arg->value.nvalue ) ) ) );
}

/**
 * [Dice] roll
 *
 * Rolls a standard dice with 6 sides.
 */
ctr_object* ctr_dice_throw(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float( (ctr_number) 1 + arc4random_uniform( (uint32_t) 6 ) );
}

/**
 * [Dice] rawRandomNumber
 *
 * Generates a random number, the traditional way (like rand()).
 */
ctr_object* ctr_dice_rand(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float( (ctr_number) (arc4random()) );
}


/**
 * [Clock] wait
 *
 * Waits X seconds.
 */
ctr_object* ctr_clock_wait(ctr_object* myself, ctr_argument* argumentList) {
	ctr_check_permission( CTR_SECPRO_COUNTDOWN );
	ctr_object* arg = ctr_internal_cast2number(argumentList->object);
	int n = (int) arg->value.nvalue;
	sleep(n);
	return myself;
}

/**
 * [Clock] time
 *
 * Returns UNIX epoch time in seconds.
 */
ctr_object* ctr_clock_time(ctr_object* myself, ctr_argument* argumentList) {
	time_t seconds = time(NULL);
	return ctr_build_number_from_float((ctr_number)seconds);
}

/**
 * [Pen] write: [String]
 *
 * Writes string to console. 
 */
ctr_object* ctr_console_write(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* argument1 = argumentList->object;
	ctr_object* strObject = ctr_internal_cast2string(argument1);
	fwrite(strObject->value.svalue->value, sizeof(char), strObject->value.svalue->vlen, stdout);
	return myself;
}

/**
 * [Pen] brk
 * 
 * Outputs a newline character.
 */
ctr_object* ctr_console_brk(ctr_object* myself, ctr_argument* argumentList) {
	fwrite("\n", sizeof(char), 1, stdout);
	return myself;
}
