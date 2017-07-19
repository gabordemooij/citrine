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
ctr_object* ctr_command_fork(ctr_object* myself, ctr_argument* argumentList) {
	int p;
	int* ps;
	FILE* pipes;
	ctr_object* child;
	ctr_argument* newArgumentList;
	ctr_resource* rs;
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
 * [Command] message: [String].
 *
 * Sends a message to another program, i.e. a child or a parent that is
 * running at the same time.
 */
ctr_object* ctr_command_message(ctr_object* myself, ctr_argument* argumentList) {
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
 * [Command] listen: [Block].
 *
 * Stops the current flow of the program and starts listening for
 * messages from other programs that are running at the same time.
 * Upon receiving a message, the specified block will be invocated
 * and passed the message that has been received.
 */
ctr_object* ctr_command_listen(ctr_object* myself, ctr_argument* argumentList) {
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
 * [Command] join
 *
 * Rejoins the program with the main program.
 * This message will cause the current program to stop and wait
 * for the child program to end.
 */
ctr_object* ctr_command_join(ctr_object* myself, ctr_argument* argumentList) {
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

ctr_object* ctr_command_pid(ctr_object* myself, ctr_argument* argumentList ) {
	ctr_object* pidObject;
	pidObject = ctr_internal_object_find_property(
		myself,
		ctr_build_string_from_cstring("pid"),
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	if (pidObject == NULL) return CtrStdNil;
	return ctr_internal_cast2number( pidObject );
}

ctr_object* ctr_command_log_generic(ctr_object* myself, ctr_argument* argumentList, int level) {
	char* message;
	message = ctr_heap_allocate_cstring(
		ctr_internal_cast2string(
			argumentList->object
		)
	);
	syslog( level, "%s", message );
	ctr_heap_free( message );
	return myself;
}

ctr_object* ctr_command_log(ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_command_log_generic( myself, argumentList, LOG_NOTICE );
}

ctr_object* ctr_command_warn(ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_command_log_generic( myself, argumentList, LOG_WARNING );
}

ctr_object* ctr_command_err(ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_command_log_generic( myself, argumentList, LOG_ERR );
}

ctr_object* ctr_command_crit(ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_command_log_generic( myself, argumentList, LOG_EMERG );
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
 * [Clock] new: [Number].
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
 * represented by the clock object.
 */
ctr_object* ctr_clock_to_string( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_argument* newArgumentList;
	ctr_object*   answer;
	newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	newArgumentList->object = ctr_build_string_from_cstring( "%Y-%m-%d %H:%M:%S" );
	answer = ctr_clock_format( myself, newArgumentList );
	ctr_heap_free( newArgumentList );
	return answer;
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
