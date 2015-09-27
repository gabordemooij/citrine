
obj* ctr_array_new(obj* myclass) {
	obj* s = ctr_internal_create_object(OTARRAY);
	s->link = myclass;
	s->value.avalue = (carray*) malloc(sizeof(carray));
	s->value.avalue->length = 1;
	s->value.avalue->elements = (obj**) malloc(sizeof(obj*)*1);
	s->value.avalue->head = 0;
	s->info.flagb = 1;
	//printf("%d\n", s->value.avalue->length);
	return s;
}

obj* ctr_array_push(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	if (myself->value.avalue->length <= (myself->value.avalue->head + 1)) {
		myself->value.avalue->length = myself->value.avalue->length * 3;
		myself->value.avalue->elements = (obj**) realloc(myself->value.avalue->elements, (sizeof(obj*) * (myself->value.avalue->length)));
	}
	obj* pushValue = argumentList->object;
	int size = sizeof(obj*);
	int offset = (myself->value.avalue->head * size);
	long nbase = (long) myself->value.avalue->elements;
	long locationa = nbase + offset;
	*((obj**)locationa) = pushValue;
	myself->value.avalue->head++;
	return myself;
}

obj* ctr_array_unshift(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	if (myself->value.avalue->length <= (myself->value.avalue->head + 1)) {
		myself->value.avalue->length = myself->value.avalue->length * 3;
		myself->value.avalue->elements = (obj**) realloc(myself->value.avalue->elements, (sizeof(obj*) * (myself->value.avalue->length)));
	}
	obj* pushValue = argumentList->object;
	myself->value.avalue->head++;
	
	memmove((obj**)((long)myself->value.avalue->elements+(sizeof(obj*))), myself->value.avalue->elements,myself->value.avalue->head*sizeof(obj*));
	*(myself->value.avalue->elements) = pushValue;
	return myself;
}



obj* ctr_array_get(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	obj* getIndex = argumentList->object;
	if (getIndex->info.type != OTNUMBER) {
		printf("Index must be number.\n"); exit(1);
	}
	int i = (int) getIndex->value.nvalue;
	if (myself->value.avalue->head < i || i < 0) {
		printf("Index out of bounds.\n"); exit(1);
	}
	return *( (obj**) ((long)myself->value.avalue->elements + (i * sizeof(obj*))) );
}

obj* ctr_array_put(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	if (!argumentList->next) {
		printf("Missing argument 2\n"); exit(1);
	}
	obj* putValue = argumentList->object;
	obj* putIndex = argumentList->next->object;
	if (putIndex->info.type != OTNUMBER) {
		printf("Index must be number.\n"); exit(1);
	}
	int i = (int) putIndex->value.nvalue;
	if (myself->value.avalue->head < i || i < 0) {
		printf("Index out of bounds.\n"); exit(1);
	}
	*( (obj**) ((long)myself->value.avalue->elements + (i * sizeof(obj*))) ) = putValue;
	return myself;
}

//@todo dont forget to gc arrays, they might hold refs to objects!
obj* ctr_array_pop(obj* myself) {
	if (myself->value.avalue->head == 0) {
		return Nil;
	}
	myself->value.avalue->head--;
	return (obj*) *((obj**)( (long) myself->value.avalue->elements + (myself->value.avalue->head * sizeof(obj*))  ));
}


obj* ctr_array_shift(obj* myself) {
	if (myself->value.avalue->head == 0) {
		return Nil;
	}
	obj* shiftedOff = *(myself->value.avalue->elements);
	myself->value.avalue->head--;
	memmove(myself->value.avalue->elements,(obj**)((long)myself->value.avalue->elements+(sizeof(obj*))),myself->value.avalue->head*sizeof(obj*));
	return shiftedOff;
}

obj* ctr_array_count(obj* myself) {
	double d = 0;
	d = (double) myself->value.avalue->head;
	return ctr_build_number_from_float( d );
}


obj* ctr_map_new(obj* myclass) {
	obj* s = ctr_internal_create_object(OTOBJECT);
	s->link = CMap;
	return s;
}



obj* ctr_map_put(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	if (!argumentList->next) {
		printf("Missing argument 2\n"); exit(1);
	}
	obj* putValue = argumentList->object;
	args* nextArgument = argumentList->next;
	if (!nextArgument->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	obj* putKey = nextArgument->object;
	
	char* key;
	long keyLen;
	
	if (putKey->info.type == OTSTRING) {
		key = calloc(putKey->value.svalue->vlen, sizeof(char));
		keyLen = putKey->value.svalue->vlen;
		memcpy(key, putKey->value.svalue->value, keyLen);
	} else {
		printf("Map key needs to be string.\n");
		exit(1);
	}
	ctr_internal_object_delete_property(myself, ctr_build_string(key, keyLen), 0);
	ctr_internal_object_add_property(myself, ctr_build_string(key, keyLen), putValue, 0);
	
    return myself;
}

obj* ctr_map_get(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	obj* searchKey = argumentList->object;
	if (searchKey->info.type != OTSTRING) {
		printf("Expected argument at: to be of type string.\n");
		exit(1);
	}
	obj* foundObject = ctr_internal_object_find_property(myself, searchKey, 0);
	if (foundObject == NULL) foundObject = ctr_build_nil();
	return foundObject;
}

obj* ctr_map_count(obj* myself) {

	
	return ctr_build_number_from_float( myself->properties->size );
}


obj* ctr_map_each(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	obj* block = argumentList->object;
	if (block->info.type != OTBLOCK) { printf("Expected code block."); exit(1); }
	block->info.sticky = 1; //mark as sticky
	cmapitem* m = myself->properties->head;
	while(m) {
		args* arguments = CTR_CREATE_ARGUMENT();
		arguments->object = m->value;
		ctr_block_run(block, arguments, myself);
		m = m->next;
	}
	block->info.mark = 0;
	block->info.sticky = 0;
	return myself;
}
