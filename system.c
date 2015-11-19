
/**
 * GarbageCollector Marker
 */
void ctr_gc_mark(ctr_object* object) {
	ctr_object* el;
	long i;
	if (object->info.type == CTR_OBJECT_TYPE_OTARRAY) {
		for (i = 0; i < object->value.avalue->head; i++) {
			el = *((ctr_object**) (long)object->value.avalue->elements+(i*sizeof(ctr_object*)) );
			el->info.mark = 1;
			ctr_gc_mark(el);
		}
	}
	ctr_mapitem* item = object->properties->head;
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
		ctr_object* o = item->value;
		ctr_object* k = item->key;
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
		gc_object_count ++;
		if (currentObject->info.mark==0 && currentObject->info.sticky==0){
			gc_dust ++;
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
void ctr_gc_collect (ctr_object* myself, ctr_argument* argumentList) {
	gc_dust = 0;
	gc_object_count = 0;
	ctr_object* context = contexts[ctr_context_id];
	int oldcid = ctr_context_id;
	while(ctr_context_id > -1) {
		ctr_gc_mark(context);
		ctr_context_id--;
		context = contexts[ctr_context_id];
	}
	ctr_gc_sweep();
	ctr_context_id = oldcid;
}

/**
 * GCDust
 *
 * Returns the number of objects collected.
 */
ctr_object* ctr_gc_dust(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float((ctr_number) gc_dust);
}

/**
 * GCCount
 *
 * Returns the number of objects marked.
 */
ctr_object* ctr_gc_object_count(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float((ctr_number) gc_object_count);
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
	memcpy(comString, arg->value.svalue->value, vlen);
	memcpy(comString+vlen,"\0",1);
	int r = system(comString);
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
ctr_object* ctr_command_num_of_args(ctr_object* myself) {
	return ctr_build_number_from_float( (ctr_number) ctr_argc );
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
ctr_object* ctr_dice_throw(ctr_object* myself) {
	return ctr_build_number_from_float( (ctr_number) (rand() % 6));
}

/**
 * CoinFlip
 *
 * Flips a coin, returns a number between 0 and 1.
 */
ctr_object* ctr_coin_flip(ctr_object* myself) {
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
