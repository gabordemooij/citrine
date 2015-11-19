
void ctr_gc_mark(ctr_object* object) {
	ctr_object* el;
	long i;
	if (object->info.type == OTARRAY) {
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

void ctr_gc_collect (ctr_object* myself, args* argumentList) {
	gc_dust = 0;
	gc_object_count = 0;
	ctr_object* context = contexts[cid];
	int oldcid = cid;
	while(cid > -1) {
		ctr_gc_mark(context);
		cid --;
		context = contexts[cid];
	}
	ctr_gc_sweep();
	cid = oldcid;
}


ctr_object* ctr_gc_dust(ctr_object* myself, args* argumentList) {
	return ctr_build_number_from_float((ctr_number) gc_dust);
}

ctr_object* ctr_gc_object_count(ctr_object* myself, args* argumentList) {
	return ctr_build_number_from_float((ctr_number) gc_object_count);
}


ctr_object* ctr_shell_call(ctr_object* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("No system argument.\n");
		exit(1);
	}
	if (argumentList->object->info.type!=OTSTRING) {
		printf("Argument of system call needs to be string\n");
		exit(1);
	}
	long vlen = argumentList->object->value.svalue->vlen;
	char* comString = malloc(vlen + 1);
	memcpy(comString, argumentList->object->value.svalue->value, vlen);
	memcpy(comString+vlen,"\0",1);
	int r = system(comString);
	return ctr_build_number_from_float( (ctr_number) r );
}


ctr_object* ctr_command_argument(ctr_object* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("No number of arg argument.\n");
		exit(1);
	}
	if (argumentList->object->info.type!=OTNUMBER) {
		printf("Argument argNo needs to be number\n");
		exit(1);
	}
	int n = (int) argumentList->object->value.nvalue;
	if (n >= __argc) return Nil;
	return ctr_build_string(__argv[n], strlen(__argv[n]));
}

ctr_object* ctr_command_num_of_args(ctr_object* myself) {
	return ctr_build_number_from_float( (ctr_number) __argc );
}



ctr_object* ctr_dice_sides(ctr_object* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("No number of sides argument.\n");
		exit(1);
	}
	if (argumentList->object->info.type!=OTNUMBER) {
		printf("Argument sides needs to be number\n");
		exit(1);
	}
	
	
	return ctr_build_number_from_float( (ctr_number) (rand() % ((int)argumentList->object->value.nvalue)));
}

ctr_object* ctr_dice_throw(ctr_object* myself) {
	return ctr_build_number_from_float( (ctr_number) (rand() % 6));
}

ctr_object* ctr_coin_flip(ctr_object* myself) {
	return ctr_build_bool((rand() % 2));
}

ctr_object* ctr_clock_wait(ctr_object* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("No brew argument.\n");
		exit(1);
	}
	if (argumentList->object->info.type!=OTNUMBER) {
		printf("Argument brew to be number\n");
		exit(1);
	}
	int n = (int) argumentList->object->value.nvalue;
	sleep(n);
	return myself;
}

ctr_object* ctr_clock_time(ctr_object* myself, args* argumentList) {
	time_t seconds = time(NULL);
	return ctr_build_number_from_float((ctr_number)seconds);
}


ctr_object* ctr_console_write(ctr_object* myself, args* argumentList) {
	ctr_object* argument1 = argumentList->object;
	ctr_object* strObject = ctr_internal_cast2string(argument1);
	fwrite(strObject->value.svalue->value, sizeof(char), strObject->value.svalue->vlen, stdout);
	return myself;
}


ctr_object* ctr_console_brk(ctr_object* myself, args* argumentList) {
	fwrite("\n", sizeof(char), 1, stdout);
	return myself;
}
