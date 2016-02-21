#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

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
		o->name = k->value.svalue->value;
		o->info.mark = 1;
		k->info.mark = 1;
		ctr_gc_mark(o);
		item = item->next;
	} 
	item = object->methods->head;
	while(item) {
		o = item->value;
		k = item->key;
		o->name = k->value.svalue->value;
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
void ctr_gc_sweep() {
	ctr_object* previousObject = NULL;
	ctr_object* currentObject = ctr_first_object;
	while(currentObject) {
		ctr_gc_object_counter ++;
		if (currentObject->info.mark==0 && currentObject->info.sticky==0){
			ctr_gc_dust_counter ++;
			if (previousObject) {
				previousObject->gnext = currentObject->gnext;
			}
			free(currentObject);
			currentObject = previousObject->gnext;
		} else {
			if (currentObject->info.mark == 1) {
				currentObject->info.mark = 0;
			}
			previousObject = currentObject;
			currentObject = currentObject->gnext;
		}
	}
}

/**
 * Broom
 * 
 * GarbageCollector, to invoke use:
 * 
 * [Broom] sweep.
 */
ctr_object* ctr_gc_collect (ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* context;
	int oldcid;
	ctr_gc_dust_counter = 0;
	ctr_gc_object_counter = 0;
	context = ctr_contexts[ctr_context_id];
	oldcid = ctr_context_id;
	while(ctr_context_id > -1) {
		ctr_gc_mark(context);
		ctr_context_id--;
		context = ctr_contexts[ctr_context_id];
	}
	ctr_gc_sweep();
	ctr_context_id = oldcid;
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
 * GCCount
 *
 * Returns the number of objects marked.
 */
ctr_object* ctr_gc_object_count(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float((ctr_number) ctr_gc_object_counter);
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
	ctr_object* arg = ctr_internal_cast2string(argumentList->object);
	long vlen = arg->value.svalue->vlen;
	char* comString = malloc(vlen + 1);
	int r;
	memcpy(comString, arg->value.svalue->value, vlen);
	memcpy(comString+vlen,"\0",1);
	r = system(comString);
	return ctr_build_number_from_float( (ctr_number) r );
}


/**
 * @internal
 *
 * Shell Object uses a fluid API.
 */
ctr_object* ctr_shell_respond_to_with(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object*   commandObj;
	ctr_object*   prefix;
	ctr_object*   suffix;
	ctr_argument* newArgumentList;
	char* command;
	int len;
	prefix = ctr_internal_cast2string(argumentList->object);
	suffix = ctr_internal_cast2string(argumentList->next->object);
	len = prefix->value.svalue->vlen + suffix->value.svalue->vlen;
	if (len == 0) return myself;
	command = (char*) malloc(len); /* actually we need +1 for the space between commands, but we dont because we remove the colon : !*/
	strncpy(command, prefix->value.svalue->value, prefix->value.svalue->vlen - 1); /* remove colon, gives room for space */
	strncpy(command + (prefix->value.svalue->vlen - 1), " ", 1); /* space to separate commands */
	strncpy(command + (prefix->value.svalue->vlen), suffix->value.svalue->value, suffix->value.svalue->vlen);
	commandObj = ctr_build_string(command, len);
	newArgumentList = CTR_CREATE_ARGUMENT();
	newArgumentList->object = commandObj;
	ctr_shell_call(myself, newArgumentList);
	return myself;
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
 * [Command] argument: [Number]
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
 * [Command] argCount
 *
 * Returns the number of CLI arguments passed to the script.
 */
ctr_object* ctr_command_num_of_args(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float( (ctr_number) ctr_argc );
}

/**
 * [Command] exit
 * 
 * Exits program immediately.
 */
ctr_object* ctr_command_exit(ctr_object* myself, ctr_argument* argumentList) {
	exit(0);
}

/**
 * [Command] env: [String]
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
	envVarNameObj = ctr_internal_cast2string(argumentList->object);
	envVarNameStr = malloc((envVarNameObj->value.svalue->vlen+1)*sizeof(char));
	strncpy(envVarNameStr, envVarNameObj->value.svalue->value, envVarNameObj->value.svalue->vlen);
	*(envVarNameStr + (envVarNameObj->value.svalue->vlen)) = '\0';
	envVal = getenv(envVarNameStr);
	if (envVal == NULL) {
		return CtrStdNil;
	}
	return ctr_build_string_from_cstring(envVal);
}

/**
 * [Command] env: [Key] val: [Value]
 *
 * Sets the value of an environment variable.
 */
ctr_object* ctr_command_set_env(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* envVarNameObj;
	ctr_object* envValObj;
	char*       envVarNameStr;
	char*       envValStr;
	envVarNameObj = ctr_internal_cast2string(argumentList->object);
	envValObj = ctr_internal_cast2string(argumentList->next->object);
	envVarNameStr = malloc((envVarNameObj->value.svalue->vlen+1)*sizeof(char));
	CTR_2CSTR(envVarNameStr, envVarNameObj);
	CTR_2CSTR(envValStr, envValObj);
	setenv(envVarNameStr, envValStr, 1);
	return myself;
}

/**
 * Command askQuestion
 *
 * Ask a question on the command-line, resumes program
 * only after pressing the enter key.
 * Only reads up to 100 characters.
 *
 * Usage:
 *
 * Pen write: 'What is your name ?'.
 * x := Command askQuestion.
 * Pen write: 'Hello ' + x + ' !', brk.
 *
 * The example above asks the user for his/her name and
 * then displays the input received.
 */
ctr_object* ctr_command_question(ctr_object* myself, ctr_argument* argumentList) {
	int c;
	ctr_size bytes = 0;
	char* buff;
	ctr_size page = 10;
	buff = malloc(page * sizeof(char));
	while ((c = getchar()) != '\n') {
		buff[bytes] = c;
		bytes++;
		if (bytes > page) {
			page *= 2;
			buff = (char*) realloc(buff, page * sizeof(char));
			if (buff == NULL) {
				CtrStdError = ctr_build_string_from_cstring("Out of memory\0");
			}
		}
	}
	return ctr_build_string(buff, bytes);
}

/**
 * [Dice] rollWithSides: [Number]
 *
 * Rolls the dice, generates a pseudo random number.
 */
ctr_object* ctr_dice_sides(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* arg = ctr_internal_cast2number(argumentList->object);
	return ctr_build_number_from_float( (ctr_number) (rand() % ((int)arg->value.nvalue)));
}

/**
 * [Dice] roll
 *
 * Rolls a standard dice with 6 sides.
 */
ctr_object* ctr_dice_throw(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float( (ctr_number) (rand() % 6));
}

/**
 * [Dice] rawRandomNumber
 *
 * Generates a random number, the traditional way (like rand()).
 */
ctr_object* ctr_dice_rand(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float( (ctr_number) (rand()) );
}


/**
 * [Clock] wait
 *
 * Waits X seconds.
 */
ctr_object* ctr_clock_wait(ctr_object* myself, ctr_argument* argumentList) {
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
