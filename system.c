
/**
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
		ctr_object* k = item->key;
		ctr_object* o = item->value;
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
 * GarbageCollector
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
 * GCDust
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
 * ShellCall
 *
 * Performs a Shell operation.
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
 * CommandArgument
 *
 * Obtains an argument from the CLI invocation.
 */
ctr_object* ctr_command_argument(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* numberObject = ctr_internal_cast2number(argumentList->object);
	int n = (int) numberObject->value.nvalue;
	if (n >= ctr_argc) return CtrStdNil;
	return ctr_build_string(ctr_argv[n], strlen(ctr_argv[n]));
}

/**
 * CommandNumberOfArguments
 *
 * Returns the number of CLI arguments passed to the script.
 */
ctr_object* ctr_command_num_of_args(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float( (ctr_number) ctr_argc );
}

/**
 * CommandQuestion
 *
 * Asks user for interactive input in CLI script.
 * Only reads up to 100 characters.
 *
 * Usage:
 *
 * answer := Command ??.
 */
ctr_object* ctr_command_question(ctr_object* myself, ctr_argument* argumentList) {
	int c;
	size_t bytes = 0;
	char* buff;
	size_t page = 10;
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
 * DiceRollWithSides
 *
 * Rolls the dice, generates a pseudo random number.
 */
ctr_object* ctr_dice_sides(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* arg = ctr_internal_cast2number(argumentList->object);
	return ctr_build_number_from_float( (ctr_number) (rand() % ((int)arg->value.nvalue)));
}

/**
 * DiceRoll
 *
 * Rolls a standard dice with 6 sides.
 */
ctr_object* ctr_dice_throw(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float( (ctr_number) (rand() % 6));
}

/**
 * CoinFlip
 *
 * Flips a coin, returns a number between 0 and 1.
 */
ctr_object* ctr_coin_flip(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool((rand() % 2));
}

/**
 * ClockWait
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
 * ClockTime
 *
 * Returns UNIX epoch time in seconds.
 */
ctr_object* ctr_clock_time(ctr_object* myself, ctr_argument* argumentList) {
	time_t seconds = time(NULL);
	return ctr_build_number_from_float((ctr_number)seconds);
}

/**
 * ConsoleWrite
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
 * ConsoleBreak
 * 
 * Outputs a newline character.
 */
ctr_object* ctr_console_brk(ctr_object* myself, ctr_argument* argumentList) {
	fwrite("\n", sizeof(char), 1, stdout);
	return myself;
}
