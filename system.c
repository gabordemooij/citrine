
void ctr_gc_mark(obj* object) {
	obj* el;
	long i;
	if (object->info.type == OTARRAY) {
		for (i = 0; i < object->value.avalue->head; i++) {
			el = *((obj**) (long)object->value.avalue->elements+(i*sizeof(obj*)) );
			el->info.mark = 1;
			ctr_gc_mark(el);
		}
	}
	ctr_mapitem* item = object->properties->head;
	while(item) {
		obj* k = item->key;
		obj* o = item->value;
		o->name = k->value.svalue->value;
		o->info.mark = 1;
		k->info.mark = 1;
		ctr_gc_mark(o);
		item = item->next;
	} 
	item = object->methods->head;
	while(item) {
		obj* o = item->value;
		obj* k = item->key;
		o->name = k->value.svalue->value;
		o->info.mark = 1;
		k->info.mark = 1;
		ctr_gc_mark(o);
		item = item->next;
	} 
}

void ctr_gc_sweep() {
	obj* previousObject = NULL;
	obj* currentObject = ctr_first_object;
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

void ctr_gc_collect (obj* myself, args* argumentList) {
	gc_dust = 0;
	gc_object_count = 0;
	obj* context = contexts[cid];
	int oldcid = cid;
	while(cid > -1) {
		ctr_gc_mark(context);
		cid --;
		context = contexts[cid];
	}
	ctr_gc_sweep();
	cid = oldcid;
}


obj* ctr_gc_dust(obj* myself, args* argumentList) {
	return ctr_build_number_from_float((ctr_number) gc_dust);
}

obj* ctr_gc_object_count(obj* myself, args* argumentList) {
	return ctr_build_number_from_float((ctr_number) gc_object_count);
}


obj* ctr_shell_call(obj* myself, args* argumentList) {
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


obj* ctr_command_argument(obj* myself, args* argumentList) {
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

obj* ctr_command_num_of_args(obj* myself) {
	return ctr_build_number_from_float( (ctr_number) __argc );
}



obj* ctr_dice_sides(obj* myself, args* argumentList) {
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

obj* ctr_dice_throw(obj* myself) {
	return ctr_build_number_from_float( (ctr_number) (rand() % 6));
}

obj* ctr_coin_flip(obj* myself) {
	return ctr_build_bool((rand() % 2));
}

obj* ctr_clock_wait(obj* myself, args* argumentList) {
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

obj* ctr_clock_time(obj* myself, args* argumentList) {
	time_t seconds = time(NULL);
	return ctr_build_number_from_float((ctr_number)seconds);
}


obj* ctr_console_write(obj* myself, args* argumentList) {
	obj* argument1 = argumentList->object;
	obj* strObject = ctr_internal_cast2string(argument1);
	fwrite(strObject->value.svalue->value, sizeof(char), strObject->value.svalue->vlen, stdout);
	return myself;
}


obj* ctr_console_brk(obj* myself, args* argumentList) {
	fwrite("\n", sizeof(char), 1, stdout);
	return myself;
}
