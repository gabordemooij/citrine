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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef forLinux
#include <bsd/stdlib.h>
#include <bsd/string.h>
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
 * [Broom] toString.
 *
 * Returns the number of objects that have been collected by the
 * garbage collector.
 */
ctr_object* ctr_gc_to_string(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_cast2string( ctr_gc_dust( myself, argumentList ) );
}

/**
 * [Program] shell: [String]
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
ctr_object* ctr_program_shell(ctr_object* myself, ctr_argument* argumentList) {
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
 * The message 'obtain' can be used to acquire the generated string.
 * The Slurp object is a separate object with minimal messages to avoid
 * 'message collision'.
 */

/**
 * [Slurp] obtain.
 * 
 * Obtains the string generated using the Slurp object.
 * A Slurp object collects all messages send to it and flushes its buffer while
 * returning the resulting string after an 'obtain' message has been received.
 * 
 * Usage:
 * 
 * Slurp hello world.
 * Pen write: (Slurp obtain).
 * 
 * This will output: 'hello world'.
 * Use the Slurp object to integrate verbose shell commands, other programming languages
 * (like SQL) etc into your main program without overusing strings.
 * 
 * Example:
 * 
 * query select: '*', from users where: 'id = 2'.
 *
 * Note that we can't use the = and * unfortunately right now
 * because = is also a method in the main object. While * can be used
 * theoretically, it expects an identifier, and 'from' is not a real
 * identifier, it's just another unary message, so instead of using a binary
 * * we simply use a keyword message select: with argument '*' and then
 * proceed our SQL query with a comma (,) to chain the rest.
 * This is an artifact of the fact that the DSL has to be embedded within
 * the language of Citrine. However even with these restrictions (some of which might be
 * alleviated in future versions) it's quite comfortable and readable to interweave
 * an external language in your Citrine script code.
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
 * [Program] argument: [Number]
 *
 * Obtains an argument from the CLI invocation.
 */
ctr_object* ctr_program_argument(ctr_object* myself, ctr_argument* argumentList) {
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
ctr_object* ctr_program_num_of_args(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float( (ctr_number) ctr_argc );
}

/**
 * [Program] exit
 * 
 * Exits program immediately.
 */
ctr_object* ctr_program_exit(ctr_object* myself, ctr_argument* argumentList) {
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
ctr_object* ctr_program_get_env(ctr_object* myself, ctr_argument* argumentList) {
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
ctr_object* ctr_program_set_env(ctr_object* myself, ctr_argument* argumentList) {
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
ctr_object* ctr_program_waitforinput(ctr_object* myself, ctr_argument* argumentList) {
	ctr_check_permission( CTR_SECPRO_COUNTDOWN );
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
				CtrStdFlow = ctr_build_string_from_cstring("Out of memory");
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
 * The input message is not allowed if message countdown has been activated
 * (Program remainingMessages:) because it might wait for content and this
 * is not allowed in a countdown sandbox.
 *
 * Usage:
 *
 * echo "hello" | ctr test.ctr
 *
 * x := Program input.
 * Pen write: x. #hello (without newline)
 *
 */
ctr_object* ctr_program_input(ctr_object* myself, ctr_argument* argumentList) {
	ctr_check_permission( CTR_SECPRO_COUNTDOWN );
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
 * @internal
 *
 * Checks whether the user is allowed to perform this kind of operation.
 */
void ctr_check_permission( uint8_t operationID ) {
	char* reason;
	if ( ( ctr_program_security_profile & operationID ) ) {
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
		if ( operationID == CTR_SECPRO_FORK ) {
			reason = "This program is not allowed to spawn other processes or serve remote objects.";
		}
		fprintf(stderr, "%s\n", reason );
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
ctr_object* ctr_program_forbid_shell( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_program_security_profile |= CTR_SECPRO_NO_SHELL;
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
ctr_object* ctr_program_forbid_file_write( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_program_security_profile |= CTR_SECPRO_NO_FILE_WRITE;
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
ctr_object* ctr_program_forbid_file_read( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_program_security_profile |= CTR_SECPRO_NO_FILE_READ;
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
ctr_object* ctr_program_forbid_include( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_program_security_profile |= CTR_SECPRO_NO_INCLUDE;
	return myself;
}

/**
 * [Program] forbidFork.
 */
ctr_object* ctr_program_forbid_fork( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_program_security_profile |= CTR_SECPRO_FORK;
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
ctr_object* ctr_program_countdown( ctr_object* myself, ctr_argument* argumentList ) {
	if ( ctr_program_security_profile & CTR_SECPRO_COUNTDOWN ) {
		fprintf(stderr, "Message quota cannot change.\n" );
		exit(1);
	}
	ctr_program_security_profile |= CTR_SECPRO_COUNTDOWN;
	ctr_program_maxtick = (uint64_t) ctr_internal_cast2number( argumentList->object )->value.nvalue;
	return myself;
}

/**
 * [Program] flush.
 *
 * Flushes the STDOUT output buffer.
 */
ctr_object* ctr_program_flush(ctr_object* myself, ctr_argument* ctr_argumentList) {
	 fflush(stdout);
	 return myself;
}

/**
 * [Program] new: [Block].
 *
 * Forks the program into two programs.
 * Creates another program that will run at the same time as the
 * current program. Both the parent and the child will obtain a reference
 * to the newly created program. The child will obtain a reference to
 * itself passed as a parameter to the code block while the parent will
 * obtain its version of the program instance as the return value of the
 * new: message.
 *
 * Note that spawning a new program will leak memory.
 * The file descriptors used to setup communication between parent and
 * child will be removed when the main program ends but any newly created
 * program will add a descriptor pair to the set. This is a limitation
 * in the current implementation.
 *
 * Usage:
 *
 * child := Program new: { :program
 * 	Pen write: 'Child', brk.
 * }.
 * Pen write: 'Parent'.
 */
ctr_object* ctr_program_fork(ctr_object* myself, ctr_argument* argumentList) {
	int p;
	int* ps;
	FILE* pipes;
	ctr_object* child;
	ctr_argument* newArgumentList;
	ctr_resource* rs;
	ctr_check_permission( CTR_SECPRO_COUNTDOWN );
	ctr_check_permission( CTR_SECPRO_FORK );
	newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	child = ctr_internal_create_object( CTR_OBJECT_TYPE_OTOBJECT );
	child->link = myself;
	ps = ctr_heap_allocate( sizeof(int) * 4 );
	pipes = ctr_heap_allocate_tracked( sizeof( FILE ) * 2 );
	rs = ctr_heap_allocate_tracked( sizeof( ctr_resource ) );
	rs->type = 2;
	rs->ptr = (void*) pipes;
	child->value.rvalue = rs;
	newArgumentList->object = child;
	pipe(ps);
	pipe(ps + 2);
	p = fork();
	if ( p < 0 ) {
		CtrStdFlow = ctr_build_string_from_cstring( "Unable to fork" );
		ctr_heap_free( newArgumentList );
		ctr_heap_free( rs );
		return CtrStdNil;
	}
	if ( p == 0 ) {
		close(*(ps + 0));
		close(*(ps + 3));
		*((FILE**)rs->ptr + 1) = fdopen( *(ps + 1), "wb" );
		*((FILE**)rs->ptr + 2) = fdopen( *(ps + 2), "rb" );
		setvbuf( *((FILE**)rs->ptr + 1), NULL, _IONBF, 0 );
		setvbuf( *((FILE**)rs->ptr + 2), NULL, _IONBF, 0 );
		rs->type = 3;
		ctr_block_runIt( argumentList->object, newArgumentList );
		fclose(*((FILE**)rs->ptr + 1));
		fclose(*((FILE**)rs->ptr + 2));
		ctr_heap_free( newArgumentList );
		ctr_heap_free( ps);
		CtrStdFlow = CtrStdExit;
		return CtrStdNil;
	} else {
		ctr_internal_object_set_property(
				child,
				ctr_build_string_from_cstring( "pid" ),
				ctr_build_number_from_float( (ctr_number) p ),
				CTR_CATEGORY_PRIVATE_PROPERTY
		);
		close(*(ps+1));
		close(*(ps+2));
		*((FILE**)rs->ptr + 3) = fdopen( *(ps + 3), "wb" );
		*((FILE**)rs->ptr + 0) = fdopen( *(ps + 0), "rb" );
		setvbuf( *((FILE**)rs->ptr + 3), NULL, _IONBF, 0 );
		setvbuf( *((FILE**)rs->ptr + 0), NULL, _IONBF, 0 );
		ctr_heap_free( newArgumentList );
		ctr_heap_free( ps );
	}
	return child;
}

/**
 * [Program] message: [String].
 *
 * Sends a message to another program, i.e. a child or a parent that is
 * running at the same time.
 */
ctr_object* ctr_program_message(ctr_object* myself, ctr_argument* argumentList) {
	char* str;
	ctr_size n;
	ctr_object* string;
	ctr_resource* rs;
	int q;
	FILE* fd;
	string = ctr_internal_cast2string( argumentList->object );
	str = ctr_heap_allocate_cstring( string );
	n = argumentList->object->value.svalue->vlen;
	q = 1;
	rs = myself->value.rvalue;
	if (rs->type == 2) q = 3;
	fd = *((FILE**)rs->ptr + q);
	fwrite( &n, sizeof(ctr_size), 1, fd);
	fwrite( str, 1, n, fd);
	ctr_heap_free(str);
	return myself;
}

/**
 * [Program] listen: [Block].
 *
 * Stops the current flow of the program and starts listening for
 * messages from other programs that are running at the same time.
 * Upon receiving a message, the specified block will be invocated
 * and passed the message that has been received.
 */
ctr_object* ctr_program_listen(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* program;
	ctr_object* answer;
	ctr_resource* r;
	ctr_argument* newArgumentList;
	int q;
	ctr_size sz;
	FILE* fd;
	char* blob;
	program = myself;
	q = 0;
	r = program->value.rvalue;
	if (r->type == 3) q = 2;
	fd = *((FILE**)r->ptr + q);
	sz = 0;
	fread(&sz, sizeof(ctr_size), 1, fd);
	blob = ctr_heap_allocate( sz );
	fread( blob, 1, sz, fd );
	newArgumentList = ctr_heap_allocate( sizeof(ctr_argument) );
	newArgumentList->object = ctr_build_string( blob, sz );
	answer = ctr_block_runIt( argumentList->object, newArgumentList );
	ctr_heap_free( blob );
	ctr_heap_free(newArgumentList);
	return answer;
}

/**
 * [Program] join
 *
 * Rejoins the program with the main program.
 * This message will cause the current program to stop and wait
 * for the child program to end.
 */
ctr_object* ctr_program_join(ctr_object* myself, ctr_argument* argumentList) {
	int pid;
	ctr_resource* rs = myself->value.rvalue;
	if (rs == NULL) return CtrStdNil;
	if (rs->type == 3) {
		CtrStdFlow = ctr_build_string_from_cstring( "a child process can not join." );
		return CtrStdNil;
	}
	pid = (int) ctr_internal_object_find_property(
		myself,
		ctr_build_string_from_cstring("pid"),
		CTR_CATEGORY_PRIVATE_PROPERTY
	)->value.nvalue;
	fclose(*((FILE**)rs->ptr + 0));
	fclose(*((FILE**)rs->ptr + 3));
	waitpid(pid, 0, 0);
	return CtrStdNil;
}

/**
 * [Program] pid
 *
 * Returns the process identification number associated with the
 * program. If the program instance refers to the currently running
 * program PID 0 will be returned.
 */
ctr_object* ctr_program_pid(ctr_object* myself, ctr_argument* argumentList ) {
	ctr_object* pidObject;
	pidObject = ctr_internal_object_find_property(
		myself,
		ctr_build_string_from_cstring("pid"),
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	if (pidObject == NULL) return CtrStdNil;
	return ctr_internal_cast2number( pidObject );
}

/**
 * [Program] toString
 *
 * Returns a string representation of the program. This will be something like
 * [PID:2833], or in case of the currently active program: [PID:0].
 */
ctr_object* ctr_program_to_string(ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_internal_cast2string(ctr_program_pid(myself, argumentList));
}

/**
 * [Program] toNumber
 *
 * Returns the program pid as a number.
 */
ctr_object* ctr_program_to_number(ctr_object* myself, ctr_argument* argumentList ) {
       return ctr_internal_cast2number(ctr_program_pid(myself, argumentList));
}


/**
 * @internal
 *
 * Internal generic logging function.
 * All logging functionality uses this function under the hood.
 */
ctr_object* ctr_program_log_generic(ctr_object* myself, ctr_argument* argumentList, int level) {
	char* message;
	ctr_check_permission( CTR_SECPRO_COUNTDOWN );
	message = ctr_heap_allocate_cstring(
		ctr_internal_cast2string(
			argumentList->object
		)
	);
	if (ctr_program_log_type == 's') {
		syslog( level, "%s", message );
	} else {
		if (level == LOG_WARNING ) {
			fwrite( CTR_ANSI_COLOR_YELLOW, sizeof(char), strlen(CTR_ANSI_COLOR_RED), stderr);
		}
		if (level == LOG_ERR || level == LOG_EMERG ) {
			fwrite( CTR_ANSI_COLOR_RED, sizeof(char), strlen(CTR_ANSI_COLOR_RED), stderr);
		}
		fwrite( message, sizeof(char), strlen(message), stderr);
		fwrite( "\n", sizeof(char), 1, stderr);
	}
	ctr_heap_free( message );
	return myself;
}

/**
 * [Program] useStandardError
 *
 * Makes the running programming use standard error for logging.
 * Default is syslog. Use this message to select the standard error channel
 * for logging. After sending this message, all messages that write log entries
 * will use the standard error channel.
 */
ctr_object* ctr_program_use_stderr(ctr_object* myself, ctr_argument* argumentList ) {
	ctr_program_log_type = 'e';
	return myself;
}

/**
 * [Program] useSyslog
 *
 * Makes the running programming use the system logger for logging.
 * This is the default. Use this message to select the syslog channel
 * for logging. After sending this message, all messages that write log entries
 * will use the system logger facility.
 */
ctr_object* ctr_program_use_syslog(ctr_object* myself, ctr_argument* argumentList ) {
	ctr_program_log_type = 's';
	return myself;
}

/**
 * [Program] log: [String]
 *
 * Logs the specified message string using syslog using log level LOG_NOTICE.
 * Use this for debugging messages or notice messages.
 */
ctr_object* ctr_program_log(ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_program_log_generic( myself, argumentList, LOG_NOTICE );
}

/**
 * [Program] warn: [String]
 *
 * Logs the specified message string using syslog using log level LOG_WARNING.
 * Use this to have your programs emit warnings.
 */
ctr_object* ctr_program_warn(ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_program_log_generic( myself, argumentList, LOG_WARNING );
}

/**
 * [Program] error: [String]
 *
 * Logs the specified message string using syslog using log level LOG_ERR.
 * Use this to log errors.
 */
ctr_object* ctr_program_err(ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_program_log_generic( myself, argumentList, LOG_ERR );
}

/**
 * [Program] alert: [String]
 *
 * Logs the specified message string using syslog using log level LOG_EMERG.
 * Use this to log critical errors or emergencies.
 */
ctr_object* ctr_program_crit(ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_program_log_generic( myself, argumentList, LOG_EMERG );
}

/**
 * Object fromComputer: [String]
 *
 * Creates a remote object from the server specified by the
 * ip address.
 */
ctr_object* ctr_program_remote(ctr_object* myself, ctr_argument* argumentList ) {
	ctr_check_permission( CTR_SECPRO_COUNTDOWN );
	ctr_object* remoteObj = ctr_internal_create_object( CTR_OBJECT_TYPE_OTOBJECT );
	remoteObj->link = CtrStdObject;
	remoteObj->info.remote = 1;
	ctr_internal_object_set_property(
		remoteObj,
		ctr_build_string_from_cstring("@"),
		ctr_internal_cast2string(
				argumentList->object
		),
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	return remoteObj;
}

/**
 * [Program] port: [Number]
 *
 * Sets the port to use for remote connections.
 */
ctr_object* ctr_program_default_port(ctr_object* myself, ctr_argument* argumentList ) {
	ctr_default_port = (uint16_t) ctr_internal_cast2number(
		argumentList->object
	)->value.nvalue;
	return myself;
}

/**
 * [Program] connectionLimit: [Number]
 *
 * Sets the maximum number of connections and requests that will be
 * accepted by the current program.
 */
ctr_object* ctr_program_accept_number(ctr_object* myself, ctr_argument* argumentList ) {
	ctr_accept_n_connections = (uint8_t) ctr_internal_cast2number(
		argumentList->object
	)->value.nvalue;
	return myself;
}

/**
 * [Program] serve: [Object]
 *
 * Serves an object. Client programs can now communicate with this object
 * and send messages to it.
 */
ctr_object* ctr_program_accept(ctr_object* myself, ctr_argument* argumentList ) {
	int listenfd =0,connfd=0;
	ctr_object* responder;
	ctr_object* answerObj;
	ctr_object* stringObj;
	ctr_object* messageDescriptorArray;
	ctr_object* messageSelector;
	ctr_argument* callArgument;
	char* dataBuff;
	size_t lengthBuff;
	struct sockaddr_in6 serv_addr;
	uint8_t x;
	ctr_check_permission( CTR_SECPRO_COUNTDOWN );
	ctr_check_permission( CTR_SECPRO_FORK );
	responder = argumentList->object;
	listenfd = socket(AF_INET6, SOCK_STREAM, 0);
	bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin6_flowinfo = 0;
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_addr = in6addr_any;
    serv_addr.sin6_port = htons(ctr_default_port);//atoi?
	bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));
	if(listen(listenfd, 10) == -1){
		CtrStdFlow = ctr_build_string_from_cstring("Unable to listen to socket.");
		return CtrStdNil;
	}
	x = 0;
	while(1 && (ctr_accept_n_connections==0 || (x < ctr_accept_n_connections)))
	{
		x++;
		connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL); // accept awaiting request
		read( connfd, &lengthBuff, sizeof(size_t));
		dataBuff = ctr_heap_allocate( lengthBuff + 1 );
		read(connfd, dataBuff, lengthBuff);
		stringObj = ctr_build_string_from_cstring(dataBuff);
		messageDescriptorArray = ctr_string_eval( stringObj, NULL );
		messageSelector = ctr_array_shift( messageDescriptorArray, NULL );
		callArgument = ctr_heap_allocate( sizeof(ctr_argument) );
		callArgument->object = messageSelector;
		callArgument->next = ctr_heap_allocate( sizeof(ctr_argument) );
		callArgument->next->object = messageDescriptorArray;
		answerObj = ctr_internal_cast2string(
			ctr_object_message( responder, callArgument )
		);
		write( connfd, (size_t*) &answerObj->value.svalue->vlen, sizeof(size_t) );
		write( connfd, answerObj->value.svalue->value, answerObj->value.svalue->vlen);
		ctr_heap_free(dataBuff);
		ctr_heap_free(callArgument->next);
		ctr_heap_free(callArgument);
		close(connfd);
	}
	shutdown(listenfd, SHUT_RDWR);
	close(listenfd);
	return 0;
}

/**
 * [Dice]
 *
 * The Dice object represents the random number generator.
 * Using a separate object for this makes it easy to spot randomness
 * in any program. The random number generator of Citrine uses the
 * BSD arc4random secure random number generator for high quality
 * randomness that is generally considered to be cryptographically
 * secure.
 */

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
 * Rolls a standard die with 6 sides.
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
 * [Dice] toNumber
 *
 * On receiving this message, the Dice instance will send the message
 * 'rawRandomNumber' to itself and return the result as a string.
 * Note that you can override this behaviour with a custom rawRandomNumber implementation.
 *
 * Usage:
 *
 * #rig the dice ;)
 * ⚄ on: 'rawRandomNumber' do: { ↲ 6. }.
 * ✎ write: ⚄, brk. #6
 * ✎ write: ⚄ toNumber, brk. #6
 */
ctr_object* ctr_dice_to_string(ctr_object* myself, ctr_argument* argumentList) {
       return ctr_internal_cast2string( ctr_send_message( myself, "rawRandomNumber", strlen("rawRandomNumber"), argumentList ) );
}

/**
 * [Dice] toNumber
 *
 * On receiving this message, the Dice instance will send the message
 * 'rawRandomNumber' to itself and return the result.
 * Note that you can override this behaviour with a custom rawRandomNumber implementation.
 *
 * Usage:
 *
 * #rig the dice ;)
 * ⚄ on: 'rawRandomNumber' do: { ↲ 6. }.
 * ✎ write: ⚄, brk. #6
 * ✎ write: ⚄ toNumber, brk. #6
 */
ctr_object* ctr_dice_to_number(ctr_object* myself, ctr_argument* argumentList) {
       return ctr_internal_cast2number( ctr_send_message( myself, "rawRandomNumber", strlen("rawRandomNumber"), argumentList ) );
}

/**
 * [Dice] drawFrom: [String] length: [Number].
 *
 * Returns a randomized string with the specified length using the pool of
 * characters contained in the String object. This message is useful for
 * creating random nonces, passwords, temporary files, keys or identifiers.
 */
ctr_object* ctr_dice_randomize_bytes(ctr_object* myself, ctr_argument* argumentList) {
	ctr_size i;
	ctr_size j;
	ctr_size k;
	ctr_size m;
	ctr_size byteOffset;
	ctr_size byteLength;
	ctr_size plen;
	ctr_size len;
	char* newBuffer;
	ctr_object* answer;
	ctr_object* pool = ctr_internal_cast2string(argumentList->object);
	plen = ctr_getutf8len( pool->value.svalue->value, pool->value.svalue->vlen );
	len = (size_t) ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
	if ( len == 0 ) return ctr_build_empty_string();
	newBuffer = (char*) ctr_heap_allocate( len * 4 );
	m = 0;
	for( i = 0; i < len; i ++ ) {
		j = (ctr_size) arc4random_uniform( (uint32_t) plen );
		byteOffset = getBytesUtf8( pool->value.svalue->value, 0, j );
		byteLength = getBytesUtf8( pool->value.svalue->value, byteOffset, 1 );
		for (k = 0; k < byteLength; k++) {
			newBuffer[m++]=*(pool->value.svalue->value+byteOffset+k);
		}
	}
	answer = ctr_build_string(newBuffer, m);
	ctr_heap_free(newBuffer);
	return answer;
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
 * whileFalse: { ⏰ wait: 1. }.
 *
 */
ctr_object* ctr_clock_wait(ctr_object* myself, ctr_argument* argumentList) {
	ctr_check_permission( CTR_SECPRO_COUNTDOWN );
	ctr_object* arg = ctr_internal_cast2number(argumentList->object);
	int n = (int) arg->value.nvalue;
	sleep(n);
	return myself;
}

/**
 * [Clock] new: [Number]
 *
 * Creates a new clock instance from a UNIX time stamp.
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
 * clock := Clock new: timeStamp.
 * copyClock := Clock new like: clock.
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
 */
ctr_object* ctr_clock_set_zone( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_internal_object_set_property( myself, ctr_build_string_from_cstring(CTR_DICT_ZONE), ctr_internal_cast2string( argumentList->object ), CTR_CATEGORY_PRIVATE_PROPERTY );
	return myself;
}

/**
 * [Clock] zone
 *
 * Returns time zone of the clock.
 */
ctr_object* ctr_clock_get_zone( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_internal_object_find_property( myself, ctr_build_string_from_cstring(CTR_DICT_ZONE), CTR_CATEGORY_PRIVATE_PROPERTY );
}

/**
 * [Clock] zone: [Number]
 *
 * Sets the year of the clock.
 */
ctr_object* ctr_clock_set_year( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_set_time( myself, argumentList, 'Y' );
}

/**
 * [Clock] zone: [Number]
 *
 * Sets the month of the clock.
 */
ctr_object* ctr_clock_set_month( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_set_time( myself, argumentList, 'm' );
}

/**
 * [Clock] zone: [Number]
 *
 * Sets the day of the clock.
 */
ctr_object* ctr_clock_set_day( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_set_time( myself, argumentList, 'd' );
}

/**
 * [Clock] zone: [Number]
 *
 * Sets the hour of the clock.
 */
ctr_object* ctr_clock_set_hour( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_set_time( myself, argumentList, 'H' );
}

/**
 * [Clock] zone: [Number]
 *
 * Sets the minute of the clock.
 */
ctr_object* ctr_clock_set_minute( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_set_time( myself, argumentList, 'i' );
}

/**
 * [Clock] zone: [Number]
 *
 * Sets the second of the clock.
 */
ctr_object* ctr_clock_set_second( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_set_time( myself, argumentList, 's' );
}

/**
 * [Clock] year
 *
 * Returns year of the clock.
 */
ctr_object* ctr_clock_year( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_get_time( myself, argumentList, 'Y' );
}

/**
 * [Clock] month
 *
 * Returns month of the clock.
 */
ctr_object* ctr_clock_month( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_get_time( myself, argumentList, 'm' );
}

/**
 * [Clock] day
 *
 * Returns day of the clock.
 */
ctr_object* ctr_clock_day( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_get_time( myself, argumentList, 'd' );
}

/**
 * [Clock] hour
 *
 * Returns hour of the clock.
 */
ctr_object* ctr_clock_hour( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_get_time( myself, argumentList, 'H' );
}

/**
 * [Clock] minute
 *
 * Returns minute of the clock.
 */
ctr_object* ctr_clock_minute( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_get_time( myself, argumentList, 'i' );
}

/**
 * [Clock] second
 *
 * Returns second of the clock.
 */
ctr_object* ctr_clock_second( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_get_time( myself, argumentList, 's' );
}

/**
 * [Clock] yearday
 *
 * Returns day number of the year.
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
 * [Clock] new time.
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
 * [Clock] toString
 *
 * Returns a string describing the date and time
 * represented by the clock object. On receiving this message, the Clock
 * instance will send the message 'format:' to itself and the argument:
 * '%Y-%m-%d %H:%M:%S'. It will return the answer as a string. Note that you
 * can override this behaviour by adding your own 'format:' implementation.
 *
 * Usage:
 *
 * #build a time machine! ;)
 * ⏰ on: 'format:' do: { ↲ 'beautiful moment'. }.
 * ⏰ on: 'time' do: { ↲ '999'. }.
 *
 * write: ⏰, brk. #beautiful moment
 * ✎ write: ⏰ toNumber, brk. #999
 */
ctr_object* ctr_clock_to_string( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_argument* newArgumentList;
	ctr_object*   answer;
	newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	newArgumentList->object = ctr_build_string_from_cstring( "%Y-%m-%d %H:%M:%S" );
	answer = ctr_send_message( myself, "format:", strlen("format:"), newArgumentList );
	ctr_heap_free( newArgumentList );
	return answer;
}

/**
 * [Clock] toNumber
 *
 * Returns a time stamp describing the date and time
 * represented by the clock object. On receiving this message, the Clock
 * instance will send the message 'time' to itself
 * and return the answer as a number. Note that you
 * can override this behaviour by adding your own 'time' implementation.
 *
 * Usage:
 *
 * #build a time machine! ;)
 * ⏰ on: 'format:' do: { ↲ 'beautiful moment'. }.
 * ⏰ on: 'time' do: { ↲ '999'. }.
 *
 * write: ⏰, brk. #beautiful moment
 * ✎ write: ⏰ toNumber, brk. #999
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
	if ( strncmp( unit, CTR_DICT_HOURS, l ) == 0 || strncmp( unit, CTR_DICT_HOUR, l ) == 0 || strncmp( unit, CTR_DICT_HOURS_ABBR, l ) == 0  ) {
		date->tm_hour += number;
	} else if ( strncmp( unit, CTR_DICT_MINUTES, l ) == 0 || strncmp( unit, CTR_DICT_MINUTE, l ) == 0 || strncmp( unit, CTR_DICT_MINUTES_ABBR, l ) == 0  ) {
		date->tm_min += number;
	} else if ( strncmp( unit, CTR_DICT_SECONDS, l ) == 0 || strncmp( unit, CTR_DICT_SECOND, l ) == 0 || strncmp( unit, CTR_DICT_SECONDS_ABBR, l ) == 0 ) {
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
 * clock add: 3 minutes. #adds 3 minutes
 * clock add: 1 hour.    #adds 1 hour
 * clock add: 2 second.  #adds 2 seconds
 */
ctr_object* ctr_clock_add( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_change( myself, argumentList, 1 );
}

/**
 * [Clock] subtract: [Number].
 *
 * Same as '[Clock] add:' but subtracts the number instead of adding it to
 * the clock's time.
 */
ctr_object* ctr_clock_subtract( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_clock_change( myself, argumentList, 0 );
}

/**
 * [Clock] new
 *
 * Creates a new clock, by default a clock will be set to
 * the UTC timezone having the current time.
 */
ctr_object* ctr_clock_new( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_object* clock;
	clock = ctr_internal_create_object( CTR_OBJECT_TYPE_OTOBJECT );
	clock->link = myself;
	ctr_clock_init( clock );
	return clock;
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

/**
 * [Pen] red
 *
 * Changes the text color of the console to red.
 */
ctr_object* ctr_console_red(ctr_object* myself, ctr_argument* argumentList) {
	fwrite( CTR_ANSI_COLOR_RED, sizeof(char), strlen(CTR_ANSI_COLOR_RED), stdout);
	return myself;
}

/**
 * [Pen] green
 *
 * Changes the text color of the console to green.
 */
ctr_object* ctr_console_green(ctr_object* myself, ctr_argument* argumentList) {
	fwrite( CTR_ANSI_COLOR_GREEN, sizeof(char), strlen(CTR_ANSI_COLOR_GREEN), stdout);
	return myself;
}

/**
 * [Pen] yellow
 *
 * Changes the text color of the console to yellow.
 */
ctr_object* ctr_console_yellow(ctr_object* myself, ctr_argument* argumentList) {
	fwrite( CTR_ANSI_COLOR_YELLOW, sizeof(char), strlen(CTR_ANSI_COLOR_YELLOW), stdout);
	return myself;
}

/**
 * [Pen] blue
 *
 * Changes the text color of the console to blue.
 */
ctr_object* ctr_console_blue(ctr_object* myself, ctr_argument* argumentList) {
	fwrite( CTR_ANSI_COLOR_BLUE, sizeof(char), strlen(CTR_ANSI_COLOR_BLUE), stdout);
	return myself;
}

/**
 * [Pen] magenta
 *
 * Changes the text color of the console to magenta.
 */
ctr_object* ctr_console_magenta(ctr_object* myself, ctr_argument* argumentList) {
	fwrite( CTR_ANSI_COLOR_MAGENTA, sizeof(char), strlen(CTR_ANSI_COLOR_MAGENTA), stdout);
	return myself;
}

/**
 * [Pen] cyan
 *
 * Changes the text color of the console to cyan.
 */
ctr_object* ctr_console_cyan(ctr_object* myself, ctr_argument* argumentList) {
	fwrite( CTR_ANSI_COLOR_CYAN, sizeof(char), strlen(CTR_ANSI_COLOR_CYAN), stdout);
	return myself;
}

/**
 * [Pen] reset
 *
 * Resets the console text color to the default.
 */
ctr_object* ctr_console_reset(ctr_object* myself, ctr_argument* argumentList) {
	fwrite( CTR_ANSI_COLOR_RESET, sizeof(char), strlen(CTR_ANSI_COLOR_RESET), stdout);
	return myself;
}

/**
 * [Pen] tab
 *
 * Prints a tab character to the console output.
 */
ctr_object* ctr_console_tab(ctr_object* myself, ctr_argument* argumentList) {
	fwrite( "\t", sizeof(char), strlen("\t"), stdout);
	return myself;
}

/**
 * [Pen] line
 *
 * Draws an ASCII line and a newline character.
 */
ctr_object* ctr_console_line(ctr_object* myself, ctr_argument* argumentList) {
	char* line = "---------------------------------------\n";
	fwrite( line, sizeof(char), strlen(line), stdout);
	return myself;
}
