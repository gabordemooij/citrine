
/**
 * ArrayNew
 *
 * Creates a new instance of an Array.
 *
 * Usage:
 *
 * a := Array new.
 */
ctr_object* ctr_array_new(ctr_object* myclass) {
	ctr_object* s = ctr_internal_create_object(OTARRAY);
	s->link = myclass;
	s->value.avalue = (carray*) malloc(sizeof(carray));
	s->value.avalue->length = 1;
	s->value.avalue->elements = (ctr_object**) malloc(sizeof(ctr_object*)*1);
	s->value.avalue->head = 0;
	s->value.avalue->tail = 0;
	s->info.flagb = 1;
	return s;
}

/**
 * ArrayPush
 *
 * Pushes an element on top of the array.
 *
 * Usage:
 *
 * numbers := Array new.
 * numbers push: 3.
 */
ctr_object* ctr_array_push(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.avalue->length <= (myself->value.avalue->head + 1)) {
		myself->value.avalue->length = myself->value.avalue->length * 3;
		myself->value.avalue->elements = (ctr_object**) realloc(myself->value.avalue->elements, (sizeof(ctr_object*) * (myself->value.avalue->length)));
	}
	ctr_object* pushValue = argumentList->object;
	*(myself->value.avalue->elements + myself->value.avalue->head) = pushValue;
	myself->value.avalue->head++;
	return myself;
}

/**
 * ArrayNewAndPush
 *
 * Creates a new instance of an array and initializes this
 * array with a first element, useful for literal-like Array
 * notations.
 *
 * Usage:
 *
 * a := Array <- 1 ; 2 ; 3.
 *
 * Note that the ; symbol here is an alias for 'push:'.
 */
ctr_object* ctr_array_new_and_push(ctr_object* myclass, ctr_argument* argumentList) {
	ctr_object* s = ctr_array_new(myclass);
	return ctr_array_push(s, argumentList);
}

ctr_object* ctr_array_unshift(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* pushValue = argumentList->object;
	if (myself->value.avalue->tail > 0) {
		myself->value.avalue->tail--;
	} else {
		if (myself->value.avalue->length <= (myself->value.avalue->head + 1)) {
			myself->value.avalue->length = myself->value.avalue->length * 3;
			myself->value.avalue->elements = (ctr_object**) realloc(myself->value.avalue->elements, (sizeof(ctr_object*) * (myself->value.avalue->length)));
		}
		myself->value.avalue->head++;
		memmove(myself->value.avalue->elements+1, myself->value.avalue->elements,myself->value.avalue->head*sizeof(ctr_object*));
	}
	*(myself->value.avalue->elements + myself->value.avalue->tail) = pushValue;
	return myself;
}

/**
 * ArrayJoin
 *
 * Joins the elements of an array together in a string
 * separated by a specified glue string.
 *
 * Usage:
 *
 * collection := Array new.
 * collection push: 1, push: 2, push 3.
 * collection join: ','. --> 1,2,3
 */
ctr_object* ctr_array_join(ctr_object* myself, ctr_argument* argumentList) {
	int i;
	char* result;
	long len = 0;
	long pos;
	ctr_object* glue = ctr_internal_cast2string(argumentList->object);
	long glen = glue->value.svalue->vlen;
	for(i=0; i<myself->value.avalue->head; i++) {
		ctr_object* o = *( myself->value.avalue->elements + i );
		ctr_object* str = ctr_internal_cast2string(o);
		pos = len;
		if (len == 0) {
			len = str->value.svalue->vlen;
			result = malloc(sizeof(char)*len);
		} else {
			len += str->value.svalue->vlen + glen;
			result = realloc(result, sizeof(char)*len);
			memcpy(result+pos, glue->value.svalue->value, glen);
			pos += glen;
		}
		memcpy(result+pos, str->value.svalue->value, str->value.svalue->vlen);
	}
	ctr_object* resultStr = ctr_build_string(result, len);
	free(result);
	return resultStr;
}

/**
 * ArrayGet
 *
 * Returns the element in the array at the specified index.
 */
ctr_object* ctr_array_get(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* getIndex = argumentList->object;
	if (getIndex->info.type != OTNUMBER) {
		printf("Index must be number.\n"); exit(1);
	}
	int i = (int) getIndex->value.nvalue;
	if (myself->value.avalue->head < i || i < 0) {
		printf("Index out of bounds.\n"); exit(1);
	}
	return *(myself->value.avalue->elements + i);
}

/**
 * ArrayPut
 *
 * Puts a value in the array at the specified index.
 */
ctr_object* ctr_array_put(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* putValue = argumentList->object;
	ctr_object* putIndex = argumentList->next->object;
	if (putIndex->info.type != OTNUMBER) {
		printf("Index must be number.\n"); exit(1);
	}
	int i = (int) putIndex->value.nvalue;
	if (myself->value.avalue->head < i || i < 0) {
		printf("Index out of bounds.\n"); exit(1);
	}
	*(myself->value.avalue->elements + i) = putValue;
	return myself;
}

/**
 * ArrayPop
 *
 * Pops off the last element of the array.
 */
//@todo dont forget to gc arrays, they might hold refs to objects!
ctr_object* ctr_array_pop(ctr_object* myself) {
	if (myself->value.avalue->tail >= myself->value.avalue->head) {
		return Nil;
	}
	myself->value.avalue->head--;
	return *(myself->value.avalue->elements + myself->value.avalue->head);
}

/**
 * ArrayShift
 *
 * Shifts off the first element of the array.
 */
ctr_object* ctr_array_shift(ctr_object* myself) {
	if (myself->value.avalue->tail >= myself->value.avalue->head) {
		return Nil;
	}	
	ctr_object* shiftedOff = *(myself->value.avalue->elements + myself->value.avalue->tail);
	myself->value.avalue->tail++;
	return shiftedOff;
}

/**
 * ArrayCount
 *
 * Returns the number of elements in the array.
 */
ctr_object* ctr_array_count(ctr_object* myself) {
	ctr_number d = 0;
	d = (ctr_number) myself->value.avalue->head - myself->value.avalue->tail;
	return ctr_build_number_from_float( (ctr_number) d );
}

/**
 * ArrayFromTo (Slice)
 *
 * Copies part of an array indicated by from and to and
 * returns a new array consisting of a copy of this region.
 */
ctr_object* ctr_array_from_to(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* startElement = ctr_internal_cast2number(argumentList->object);
	ctr_object* count = ctr_internal_cast2number(argumentList->next->object);
	int start = (int) startElement->value.nvalue;
	int len = (int) count->value.nvalue;
	int i = 0;
	ctr_object* newArray = ctr_array_new(CArray);
	for(i = start; i < start + len; i++) {
		ctr_argument* pushArg = CTR_CREATE_ARGUMENT();
		ctr_argument* elnumArg = CTR_CREATE_ARGUMENT();
		ctr_object* elnum = ctr_build_number_from_float((ctr_number) i);
		elnumArg->object = elnum;
		pushArg->object = ctr_array_get(myself, elnumArg);
		ctr_array_push(newArray, pushArg);
	}
	return newArray;
}

/**
 * ArrayAdd (+)
 *
 * Returns a new array, containing elements of itself and the other
 * array.
 */
ctr_object* ctr_array_add(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherArray = argumentList->object;
	ctr_object* newArray = ctr_array_new(CArray);
	int i;
	for(i = myself->value.avalue->tail; i<myself->value.avalue->head; i++) {
		ctr_argument* pushArg = CTR_CREATE_ARGUMENT();
		ctr_argument* elnumArg = CTR_CREATE_ARGUMENT();
		ctr_object* elnum = ctr_build_number_from_float((ctr_number) i);
		elnumArg->object = elnum;
		pushArg->object = ctr_array_get(myself, elnumArg);
		ctr_array_push(newArray, pushArg);
	}
	if (otherArray->info.type == OTARRAY) {
		for(i = otherArray->value.avalue->tail; i<otherArray->value.avalue->head; i++) {
			ctr_argument* pushArg = CTR_CREATE_ARGUMENT();
			ctr_argument* elnumArg = CTR_CREATE_ARGUMENT();
			ctr_object* elnum = ctr_build_number_from_float((ctr_number) i);
			elnumArg->object = elnum;
			pushArg->object = ctr_array_get(otherArray, elnumArg);
			ctr_array_push(newArray, pushArg);
		}
	}
	return newArray;
}

/**
 * StringSplit
 *
 * Converts a string to an array by splitting the string using
 * the specified delimiter (also a string).
 */
ctr_object* ctr_string_split(ctr_object* myself, ctr_argument* argumentList) {
	char* str = myself->value.svalue->value;
	long len = myself->value.svalue->vlen;
	ctr_object* delimObject  = ctr_internal_cast2string(argumentList->object);
	char* dstr = delimObject->value.svalue->value;
	long dlen = delimObject->value.svalue->vlen;
	ctr_object* arr = ctr_array_new(CArray);
	long i;
	long j = 0;
	char* buffer = malloc(sizeof(char)*len);
	for(i=0; i<len; i++) {
		buffer[j] = str[i];
		j++;
		//found a delimiter
		if (memmem(buffer, j, dstr, dlen)!=NULL) {
			//take part before delim
			char* elem = malloc(sizeof(char)*(j-dlen));
			memcpy(elem,buffer,j-dlen);
			//put it in array
			ctr_argument* arg = malloc(sizeof(ctr_argument));
			arg->object = ctr_build_string(elem, j-dlen);
			ctr_array_push(arr, arg);
			free(arg);
			j=0;
		}
	}
	if (j>0) {
		//put remainder in array
		char* elem = malloc(sizeof(char)*j);
		memcpy(elem,buffer,j);
		//put it in array
		ctr_argument* arg = malloc(sizeof(ctr_argument));
		arg->object = ctr_build_string(elem, j);
		ctr_array_push(arr, arg);
		free(arg);
	}
	free(buffer);
	return arr;
}

/**
 * Internal sort function, for use with ArraySort.
 * Interfaces with qsort-compatible function.
 */
ctr_object* temp_sorter;
int ctr_sort_cmp(const void * a, const void * b) {
	ctr_argument* arg1 = CTR_CREATE_ARGUMENT();
	ctr_argument* arg2 = CTR_CREATE_ARGUMENT();
	arg1->next = arg2;
	arg1->object = *((ctr_object**) a);
	arg2->object = *((ctr_object**) b);
	ctr_object* result = ctr_block_run(temp_sorter, arg1, temp_sorter);
	ctr_object* numResult = ctr_internal_cast2number(result);
	return (int) numResult->value.nvalue;
}

/**
 * ArraySort
 *
 * Sorts the contents of an array using a sort block.
 * Uses qsort.
 */
ctr_object* ctr_array_sort(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* sorter = argumentList->object;
	if (sorter->info.type != OTBLOCK) {
		error = ctr_build_string_from_cstring("Expected block.\0");
		return myself;
	}
	temp_sorter = sorter;
	qsort(myself->value.avalue->elements, myself->value.avalue->head, sizeof(ctr_object*), ctr_sort_cmp);
	return myself;
}

ctr_object* ctr_map_new(ctr_object* myclass) {
	ctr_object* s = ctr_internal_create_object(OTOBJECT);
	s->link = CMap;
	return s;
}



ctr_object* ctr_map_put(ctr_object* myself, ctr_argument* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	if (!argumentList->next) {
		printf("Missing argument 2\n"); exit(1);
	}
	ctr_object* putValue = argumentList->object;
	ctr_argument* nextArgument = argumentList->next;
	if (!nextArgument->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	ctr_object* putKey = nextArgument->object;
	
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

ctr_object* ctr_map_get(ctr_object* myself, ctr_argument* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	ctr_object* searchKey = argumentList->object;
	if (searchKey->info.type != OTSTRING) {
		printf("Expected argument at: to be of type string.\n");
		exit(1);
	}
	ctr_object* foundObject = ctr_internal_object_find_property(myself, searchKey, 0);
	if (foundObject == NULL) foundObject = ctr_build_nil();
	return foundObject;
}

ctr_object* ctr_map_count(ctr_object* myself) {

	
	return ctr_build_number_from_float( (ctr_number) myself->properties->size );
}


ctr_object* ctr_map_each(ctr_object* myself, ctr_argument* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	ctr_object* block = argumentList->object;
	if (block->info.type != OTBLOCK) { printf("Expected code block."); exit(1); }
	block->info.sticky = 1; //mark as sticky
	ctr_mapitem* m = myself->properties->head;
	while(m) {
		ctr_argument* arguments = CTR_CREATE_ARGUMENT();
		arguments->object = m->value;
		ctr_block_run(block, arguments, myself);
		m = m->next;
	}
	block->info.mark = 0;
	block->info.sticky = 0;
	return myself;
}

