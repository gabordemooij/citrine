#include "citrine.h"

/**
 * @def
 * List
 * 
 * @example
 * ☞ a ≔
 *	List new
 *	• ‘aaa’
 *	• ‘bbb’
 *	• ‘ccc’.
 *
 * ☞ b ≔ List ← ‘x’ ; ‘y’.
 * ✎ write: a, stop.
 * ✎ write: b, stop.
 */
ctr_object* ctr_array_new(ctr_object* myclass, ctr_argument* argumentList) {
	ctr_object* s = ctr_internal_create_object(CTR_OBJECT_TYPE_OTARRAY);
	s->link = myclass;
	s->value.avalue = (ctr_collection*) ctr_heap_allocate(sizeof(ctr_collection));
	s->value.avalue->length = 1;
	s->value.avalue->elements = (ctr_object**) ctr_heap_allocate(sizeof(ctr_object*)*1);
	s->value.avalue->head = 0;
	s->value.avalue->tail = 0;
	return s;
}

/**
 * @def
 * [ List ] type
 *
 * @example
 * ☞ x ≔ List new.
 * ☞ y ≔ x type.
 * ✎ write: y, stop.
 */
ctr_object* ctr_array_type(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_string_from_cstring( CTR_DICT_ARRAY_OBJECT );
}

/**
 * @def
 * [ List ] append: [ String ]
 * 
 * @example
 * ☞ x ≔ List new.
 * x append: 3.
 * x ; 3.
 * x • 3.
 * ✎ write: x, stop.
 */
ctr_object* ctr_array_push(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* pushValue;
	if (myself->value.avalue->length <= (myself->value.avalue->head + 1)) {
		myself->value.avalue->length = myself->value.avalue->length * 3;
		myself->value.avalue->elements = (ctr_object**) ctr_heap_reallocate(myself->value.avalue->elements,
			(sizeof(ctr_object*) * (myself->value.avalue->length))
		);
	}
	pushValue = argumentList->object;
	*(myself->value.avalue->elements + myself->value.avalue->head) = pushValue;
	myself->value.avalue->head++;
	return myself;
}

/**
 * @def
 * [ List ] minimum
 *
 * @example
 * ☞ x ≔ List ← 8 ; 4 ; 2 ; 16.
 * ☞ y ≔ x minimum.
 * ✎ write: y, stop.
 */
ctr_object* ctr_array_min(ctr_object* myself, ctr_argument* argumentList) {
	double min = 0;
	double v = 0;
	ctr_object* el;
	size_t i = 0;
	for(i = 0; i < myself->value.avalue->head; i++) {
		el = *(myself->value.avalue->elements + i);
		v = ctr_internal_cast2number(el)->value.nvalue;
		if (i == 0 || v < min) {
			min = v;
		}
	}
	return ctr_build_number_from_float(min);
}

/**
 * @def
 * [ List ] maximum
 *
 * @example
 * ☞ x ≔ List ← 8 ; 4 ; 2 ; 16.
 * ☞ y ≔ x maximum.
 * ✎ write: y, stop.
 */
ctr_object* ctr_array_max(ctr_object* myself, ctr_argument* argumentList) {
	double max = 0;
	double v = 0;
	ctr_object* el;
	size_t i = 0;
	for(i = 0; i < myself->value.avalue->head; i++) {
		el = *(myself->value.avalue->elements + i);
		v = ctr_internal_cast2number(el)->value.nvalue;
		if (i == 0 || max < v) {
			max = v;
		}
	}
	return ctr_build_number_from_float(max);
}

/**
 * @def
 * [ List ] each: [ Block ]
 *
 * @example
 * ☞ x ≔ List ← 1 ; 2 ; 3.
 * x each: { :x
 *   ✎ write: x, stop.
 * }.
 */
ctr_object* ctr_array_map(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* block = argumentList->object;
	ctr_size i = 0;
	ctr_size j = 0;
	if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
		CtrStdFlow = ctr_error( CTR_ERR_EXP_BLK, 0 );
		return myself;
	}
	for(i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
		j++;
		ctr_argument* arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		ctr_argument* argument2 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		ctr_argument* argument3 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		arguments->object = ctr_build_number_from_float((double) j);
		argument2->object = *(myself->value.avalue->elements + i);
		argument3->object = myself;
		arguments->next = argument2;
		argument2->next = argument3;
		/* keep receiver in block object otherwise, GC will destroy it */
		ctr_gc_internal_pin(block);
		ctr_gc_internal_pin(myself);
		ctr_gc_internal_pin(argument2->object);
		ctr_block_run(block, arguments, NULL);
		ctr_heap_free( arguments );
		ctr_heap_free( argument2 );
		ctr_heap_free( argument3 );
		if (CtrStdFlow == CtrStdContinue) CtrStdFlow = NULL;
		if (CtrStdFlow) break;
	}
	if (CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL; /* consume break */
	return myself;
}

/**
 * @def
 * [ List ] ← [ Object ]
 *
 * @example
 * ☞ x ≔ List ← 1 ; ‘2’ ; False ; Nil.
 * ✎ write: x, stop.
 */
ctr_object* ctr_array_new_and_push(ctr_object* myclass, ctr_argument* argumentList) {
	ctr_object* s = ctr_array_new(myclass, NULL);
	return ctr_array_push(s, argumentList);
}

/**
 * @def
 * [ List ] prepend: [ String ]
 * 
 * @example
 * ☞ x ≔ List ← 1 ; 2 ; 3.
 * x append: 0.
 * x prepend: 9.
 * ✎ write: x, stop.
 */
ctr_object* ctr_array_unshift(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* pushValue = argumentList->object;
	if (myself->value.avalue->tail > 0) {
		myself->value.avalue->tail--;
	} else {
		if (myself->value.avalue->length <= (myself->value.avalue->head + 1)) {
			myself->value.avalue->length = myself->value.avalue->length * 3;
			myself->value.avalue->elements = (ctr_object**) ctr_heap_reallocate(myself->value.avalue->elements, (sizeof(ctr_object*) * (myself->value.avalue->length)));
		}
		myself->value.avalue->head++;
		memmove(myself->value.avalue->elements+1, myself->value.avalue->elements,myself->value.avalue->head*sizeof(ctr_object*));
	}
	*(myself->value.avalue->elements + myself->value.avalue->tail) = pushValue;
	return myself;
}

/**
 * @def
 * [ List ] join: [ String ]
 *
 * @example
 * ☞ x ≔ List ← 1 ; 2 ; 3.
 * ✎ write: (x join: ‘,’), stop.
 */
ctr_object* ctr_array_join(ctr_object* myself, ctr_argument* argumentList) {
	ctr_size i;
	char* result = NULL;
	ctr_size len = 0;
	ctr_size pos;
	ctr_object* o;
	ctr_object* str;
	ctr_object* resultStr;
	ctr_object* glue = ctr_internal_cast2string(argumentList->object);
	ctr_size glen = glue->value.svalue->vlen;
	for(i=myself->value.avalue->tail; i<myself->value.avalue->head; i++) {
		o = *( myself->value.avalue->elements + i );
		str = ctr_internal_cast2string(o);
		pos = len;
		if (i == myself->value.avalue->tail) {
			len = str->value.svalue->vlen;
			result = ctr_heap_allocate(sizeof(char)*len);
		} else {
			len += str->value.svalue->vlen + glen;
			result = ctr_heap_reallocate(result, sizeof(char)*len );
			memcpy(result+pos, glue->value.svalue->value, glen);
			pos += glen;
		}
		memcpy(result+pos, str->value.svalue->value, str->value.svalue->vlen);
	}
	resultStr = ctr_build_string(result, len);
	if (i > myself->value.avalue->tail) ctr_heap_free( result );
	return resultStr;
}

/**
 * @def
 * [ List ] position: [ Number ]
 *
 * @example
 * ☞ x  ≔ List ← ‘A’ ; ‘B’ ; ‘C’.
 * ☞ y  ≔ x position: 1.
 * ☞ z  ≔ x ? 2.
 * ✎ write: y, stop.
 * ✎ write: z, stop.
 */
ctr_object* ctr_array_get(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* getIndex = argumentList->object;
	int i;
	if (getIndex->info.type != CTR_OBJECT_TYPE_OTNUMBER) {
		CtrStdFlow = ctr_error( CTR_ERR_EXP_NUM, 0 );
		return CtrStdNil;
	}
	i = (int) getIndex->value.nvalue - 1;
	if (myself->value.avalue->head <= (i + myself->value.avalue->tail) || i < 0) {
		return CtrStdNil;
	}
	ctr_object* q =  *(myself->value.avalue->elements + myself->value.avalue->tail + i);
	return q;
}

/**
 * @def
 * [ List ] first
 *
 * @example
 * ☞ x ≔ List ← 1 ; 2 ; 3 ; 4.
 * ☞ y ≔ x first.
 * ✎ write: x, stop.
 * ✎ write: y, stop.
 */
ctr_object* ctr_array_first(ctr_object* myself, ctr_argument* argumentList) {
	ctr_size length = 0;
	length = (ctr_size) myself->value.avalue->head - myself->value.avalue->tail;
	if ( length < 1 ) {
		return CtrStdNil;
	}
	return *(myself->value.avalue->elements + myself->value.avalue->tail);
}

/**
 * @def
 * [ List ] last
 *
 * @example
 * ☞ x ≔ List ← 1 ; 2 ; 3 ; 4.
 * ☞ y ≔ x last.
 * ✎ write: x, stop.
 * ✎ write: y, stop.
 */
ctr_object* ctr_array_last(ctr_object* myself, ctr_argument* argumentList) {
	ctr_size length = 0;
	length = (ctr_size) myself->value.avalue->head - myself->value.avalue->tail;
	if ( length < 1 ) {
		return CtrStdNil;
	}
	return *(myself->value.avalue->elements + myself->value.avalue->tail + (length - 1));
}

/**
 * @def
 * [ List ] second last
 *
 * @example
 * ☞ x ≔ List ← 1 ; 2 ; 3 ; 4.
 * ☞ y ≔ x second last.
 * ✎ write: x, stop.
 * ✎ write: y, stop.
 */
ctr_object* ctr_array_second_last(ctr_object* myself, ctr_argument* argumentList) {
	ctr_size length = 0;
	length = (ctr_size) myself->value.avalue->head - myself->value.avalue->tail;
	if ( length < 2 ) {
		return CtrStdNil;
	}
	return *(myself->value.avalue->elements + myself->value.avalue->tail + (length - 2));
}


/**
 * @def
 * [ List ] put: [ Object ] at: [ Number ]
 *
 * @example
 * ☞ x ≔ List new.
 * ☞ x put: ‘a’ at: 5.
 * ✎ write: x, stop.
 */
ctr_object* ctr_array_put(ctr_object* myself, ctr_argument* argumentList) {

	ctr_object* putValue = argumentList->object;
	ctr_object* putIndex = ctr_internal_cast2number(argumentList->next->object);
	ctr_size putIndexNumber;
	ctr_size head;
	ctr_size tail;

	if (putIndex->value.nvalue < 1) {
		CtrStdFlow = ctr_error( CTR_ERR_BOUNDS, 0 );
		return myself;
	}

	head = (ctr_size) myself->value.avalue->head;
	tail = (ctr_size) myself->value.avalue->tail;
	putIndexNumber = (ctr_size) putIndex->value.nvalue - 1;
	if (head <= putIndexNumber) {
		ctr_size j;
		for(j = head; j <= putIndexNumber; j++) {
			ctr_argument* argument;
			argument = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
			argument->object = CtrStdNil;
			ctr_array_push(myself, argument);
			ctr_heap_free( argument );
		}
		myself->value.avalue->head = putIndexNumber + 1;
	}
	if (putIndexNumber < tail) {
		ctr_size j;
		for(j = tail; j > putIndexNumber; j--) {
			*(myself->value.avalue->elements + j) = CtrStdNil;
		}
		myself->value.avalue->tail = putIndexNumber;
	}
	*(myself->value.avalue->elements + putIndexNumber) = putValue;
	return myself;
}

/**
 * @def
 * [ List ] pop
 *
 * @example
 * ☞ x ≔ List ← 1 ; 2 ; 3.
 * ✎ write: x, stop.
 * x pop.
 * ✎ write: x, stop.
 */
ctr_object* ctr_array_pop(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.avalue->tail >= myself->value.avalue->head) {
		return CtrStdNil;
	}
	myself->value.avalue->head--;
	return *(myself->value.avalue->elements + myself->value.avalue->head);
}

/**
 * @def
 * [ List ] - [ Number ]
 *
 * @example
 * ☞ x ≔ List ← 1 ; 2 ; 3.
 * ✎ write: x, stop.
 * ☞ x - 1.
 * ✎ write: x, stop.
 */
ctr_object* ctr_array_delete(ctr_object* myself, ctr_argument* argumentList) {
	ctr_size index = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	ctr_size length = (ctr_size) myself->value.avalue->head - myself->value.avalue->tail;
	ctr_size i;
	ctr_size found = 0;
	index--;
	for( i = index; i < length; i ++ ) {
		*(myself->value.avalue->elements + i) = *(myself->value.avalue->elements + (i+1));
		found = 1;
	}
	if (found) {
		myself->value.avalue->head--;
	}
	return myself;
}

/**
 * @def
 * [ List ] shift
 *
 * @example
 * ☞ x ≔ List ← 1 ; 2 ; 3.
 * ☞ y ≔ x shift.
 * ✎ write: y, stop.
 * ✎ write: x, stop.
 */
ctr_object* ctr_array_shift(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* shiftedOff;
	if (myself->value.avalue->tail >= myself->value.avalue->head) {
		return CtrStdNil;
	}
	shiftedOff = *(myself->value.avalue->elements + myself->value.avalue->tail);
	myself->value.avalue->tail++;
	return shiftedOff;
}

/**
 * @def
 * [ List ] count
 * 
 * @example
 * ☞ x ≔ List ← 1 ; 2 ; 3.
 * ✎ write: x count, stop.
 */
ctr_object* ctr_array_count(ctr_object* myself, ctr_argument* argumentList) {
	ctr_number d = 0;
	d = (ctr_number) myself->value.avalue->head - myself->value.avalue->tail;
	return ctr_build_number_from_float( (ctr_number) d );
}

/**
 * @def
 * [ List ] from: [ Number ] length: [ Number ]
 * 
 * @example
 * ☞ x ≔ List ← 1 ; 2 ; 3 ; 4 ; 5.
 * ☞ y ≔ x from: 2 length: 2.
 * ✎ write: y, stop.
 */
ctr_object* ctr_array_from_length(ctr_object* myself, ctr_argument* argumentList) {
	ctr_argument* pushArg;
	ctr_argument* elnumArg;
	ctr_object* elnum;
	ctr_object* startElement = ctr_internal_cast2number(argumentList->object);
	ctr_object* count = ctr_internal_cast2number(argumentList->next->object);
	int start = (int) startElement->value.nvalue;
	int len = (int) count->value.nvalue;
	int i = 0;
	ctr_object* newArray = ctr_array_new(CtrStdArray, NULL);
	for(i = start; i < start + len; i++) {
		pushArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		elnumArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		elnum = ctr_build_number_from_float((ctr_number) i);
		elnumArg->object = elnum;
		pushArg->object = ctr_array_get(myself, elnumArg);
		ctr_array_push(newArray, pushArg);
		ctr_heap_free( elnumArg );
		ctr_heap_free( pushArg );
	}
	return newArray;
}

/**
 * @def
 * [ List ] replace: [ Number ] length: [ Number ] with: [ List ]
 * 
 * @example
 * ☞ x ≔ List ← 1 ; 2 ; 3 ; 4 ; 5.
 * ☞ z ≔ List ← 9.
 * ☞ y ≔ x replace: 2 length: 1 with: z.
 * ✎ write: y, stop.
 */
ctr_object* ctr_array_splice(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* newArray = ctr_array_new(CtrStdArray, NULL);
	
	ctr_object* fromObject        = ctr_internal_cast2number(argumentList->object);
	ctr_object* lengthObject      = ctr_internal_cast2number(argumentList->next->object);
	ctr_object* replacement       = argumentList->next->next->object;
	ctr_number from   = fromObject->value.nvalue;
	ctr_number length = lengthObject->value.nvalue;
	ctr_object* remainder; 
	ctr_argument* sliceFromArg;
	ctr_argument* sliceLengthArg;
	ctr_argument* replacementArg;
	ctr_argument* remainderArg;
	if ( replacement->info.type != CTR_OBJECT_TYPE_OTARRAY ) {
		CtrStdFlow = ctr_error( CTR_ERR_EXP_ARR, 0 );
		return myself;
	}
	sliceFromArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	sliceLengthArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	replacementArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	remainderArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	sliceFromArg->object   = ctr_build_number_from_float(1);
	sliceLengthArg->object = ctr_build_number_from_float(from - 1);
	sliceFromArg->next = sliceLengthArg;
	newArray = ctr_array_from_length( myself, sliceFromArg );
	replacementArg->object = replacement;
	newArray = ctr_array_add(newArray, replacementArg);
	sliceFromArg->object = ctr_build_number_from_float( from + length  );
	ctr_object* totalLength = ctr_array_count( myself, NULL );
	if (sliceFromArg->object->value.nvalue <= totalLength->value.nvalue) {
		sliceLengthArg->object = ctr_build_number_from_float( 1 + totalLength->value.nvalue - sliceFromArg->object->value.nvalue );
		sliceFromArg->next = sliceLengthArg;
		remainder = ctr_array_from_length( myself, sliceFromArg );
		remainderArg->object = remainder;
		newArray = ctr_array_add( newArray, remainderArg );
	}
	ctr_heap_free( sliceFromArg );
	ctr_heap_free( sliceLengthArg );
	ctr_heap_free( replacementArg );
	ctr_heap_free( remainderArg );
	return newArray;
}

/**
 * @def
 * [ List ] + [ List ]
 *
 * @example
 * ☞ x ≔ List ← 1 ; 2.
 * ☞ y ≔ List ← 3 ; 4.
 * ☞ z ≔ x + y.
 * ✎ write: z, stop.
 */
ctr_object* ctr_array_add(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherArray = argumentList->object;
	ctr_object* newArray = ctr_array_new(CtrStdArray, NULL);
	ctr_size i;
	for(i = myself->value.avalue->tail; i<myself->value.avalue->head; i++) {
		ctr_argument* pushArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		ctr_argument* elnumArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		ctr_object* elnum = ctr_build_number_from_float((ctr_number) i + 1);
		elnumArg->object = elnum;
		pushArg->object = ctr_array_get(myself, elnumArg);
		ctr_array_push(newArray, pushArg);
		ctr_heap_free( elnumArg );
		ctr_heap_free( pushArg );
	}
	if (otherArray->info.type == CTR_OBJECT_TYPE_OTARRAY) {
		for(i = otherArray->value.avalue->tail; i<otherArray->value.avalue->head; i++) {
			ctr_argument* pushArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
			ctr_argument* elnumArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
			ctr_object* elnum = ctr_build_number_from_float((ctr_number) i + 1);
			elnumArg->object = elnum;
			pushArg->object = ctr_array_get(otherArray, elnumArg);
			ctr_array_push(newArray, pushArg);
			ctr_heap_free( elnumArg );
			ctr_heap_free( pushArg );
		}
	}
	return newArray;
}

/**
 * @def
 * [ List ] by: [ List ]
 *
 * @example
 * ☞ x ≔ List ← ‘A’ ; ‘B’ ; ‘C’.
 * ☞ y ≔ List ← 1 ; 2 ; 3.
 * ☞ z ≔ x by: y.
 * ✎ write: z, stop.
 */
ctr_object* ctr_array_combine(ctr_object* myself, ctr_argument* argumentList) {
	ctr_size i;
	ctr_object* map = ctr_map_new( CtrStdMap, argumentList );
	if (argumentList->object->info.type != CTR_OBJECT_TYPE_OTARRAY) {
		return map;
	}
	ctr_argument* key   = ctr_heap_allocate( sizeof( ctr_argument ) );
	ctr_argument* value = ctr_heap_allocate( sizeof( ctr_argument ) );
	ctr_argument* index = ctr_heap_allocate( sizeof( ctr_argument ) );
	for(i = myself->value.avalue->tail; i<myself->value.avalue->head; i++) {
			index->object = ctr_build_number_from_float((ctr_number) i + 1);
			key->object = ctr_array_get( myself, index );
			value->object = ctr_array_get( argumentList->object, index );
			key->next = value;
			ctr_send_message( map, CTR_DICT_PUT_AT, strlen(CTR_DICT_PUT_AT), key);
			ctr_map_put( map, key );
	}
	ctr_heap_free(key);
	ctr_heap_free(value);
	ctr_heap_free(index);
	return map;
}

/**
 * @def
 * [ List ] copy
 *
 * @example
 * ☞ a ≔ List ← 1 ; 2 ; 3.
 * ☞ b ≔ a copy.
 * b put: 999 at: 1.
 * ✎ write: a, stop.
 * ✎ write: b, stop.
 */
ctr_object* ctr_array_copy(ctr_object* myself, ctr_argument* argumentList) {
	ctr_size i = 0;
	ctr_object* copy = ctr_array_new( CtrStdArray, argumentList );
	ctr_argument* arg = ctr_heap_allocate(sizeof(ctr_argument));
	ctr_argument* index   = ctr_heap_allocate( sizeof( ctr_argument ) );
	for(i = myself->value.avalue->tail; i<myself->value.avalue->head; i++) {
		index->object = ctr_build_number_from_float((ctr_number) i + 1);
		arg->object = ctr_array_get( myself, index );
		ctr_array_push( copy, arg );
	}
	ctr_heap_free( arg );
	ctr_heap_free( index );
	return copy;
}

/**
 * @internal
 *
 * Internal sort function, for use with ArraySort.
 * Interfaces with qsort-compatible function.
 */
ctr_object* temp_sorter;
ctr_object* temp_self;
int ctr_sort_cmp(const void * a, const void * b) {
	ctr_argument* arg1 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	ctr_argument* arg2 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	ctr_object* result;
	ctr_object* numResult;
	arg1->next = arg2;
	arg1->object = *((ctr_object**) a);
	arg2->object = *((ctr_object**) b);
	ctr_gc_internal_pin(temp_sorter);
	ctr_gc_internal_pin(temp_self);
	ctr_gc_internal_pin(arg1->object);
	ctr_gc_internal_pin(temp_self);
	result = ctr_block_run(temp_sorter, arg1, NULL);
	numResult = ctr_internal_cast2number(result);
	ctr_heap_free( arg1 );
	ctr_heap_free( arg2 );
	return (int) numResult->value.nvalue;
}

/**
 * @def
 * [ List ] sort: [ Block ]
 *
 * @example
 * ☞ x ≔ List ← 2 ; 1 ; 3.
 * x sort: { :a :b ↲ a < b.}.
 * ✎ write: x, stop.
 */
ctr_object* ctr_array_sort(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* sorter = argumentList->object;
	if (sorter->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
		CtrStdFlow = ctr_error( CTR_ERR_EXP_BLK, 0 );
		return myself;
	}
	temp_sorter = sorter;
	temp_self = myself;
	qsort((myself->value.avalue->elements+myself->value.avalue->tail), myself->value.avalue->head-myself->value.avalue->tail, sizeof(ctr_object*), ctr_sort_cmp);
	return myself;
}

/**
 * @def
 * [ List ] string
 * 
 * @example
 * ☞ x ≔ List ← ‘a’ ; ‘b’ ; ‘c’.
 * ☞ y ≔ x string.
 * ✎ write: y, stop.
 */
ctr_object* ctr_array_to_string( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_size i;
	ctr_object* arrayElement;
	ctr_argument* newArgumentList;
	static uint8_t call_depth = 0;
	call_depth++;
	if (call_depth>99) {
		CtrStdFlow = ctr_error( CTR_ERR_NESTING, 0 );
		return ctr_build_empty_string();
	}
	ctr_object* string = ctr_build_empty_string();
	ctr_gc_internal_pin(string);
	newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	if ( myself->value.avalue->tail == myself->value.avalue->head ) {
		newArgumentList->object = ctr_build_string_from_cstring( CTR_DICT_CODEGEN_ARRAY_NEW );
		string = ctr_string_append( string, newArgumentList );
	} else {
		newArgumentList->object = ctr_build_string_from_cstring( CTR_DICT_CODEGEN_ARRAY_NEW_PUSH );
		string = ctr_string_append( string, newArgumentList );
	}
	for(i=myself->value.avalue->tail; i<myself->value.avalue->head; i++) {
		arrayElement = *( myself->value.avalue->elements + i );
		newArgumentList->object = CtrStdNil;
		newArgumentList->object = ctr_send_message( arrayElement, CTR_DICT_CODE, strlen(CTR_DICT_CODE), newArgumentList );
		string = ctr_string_append( string, newArgumentList );
		if (  (i + 1 )<myself->value.avalue->head ) {
			newArgumentList->object = ctr_build_string_from_cstring(" ; ");
			string = ctr_string_append( string, newArgumentList );
		}
	}
	ctr_heap_free( newArgumentList );
	call_depth--;
	return string;
}

/**
 * @def
 * [ List ] fill: [ Number ] with: [ Object ]
 *
 * @example
 * ☞ x ≔ List new
 * fill: 10 with: ‘X’.
 * ✎ write: x, stop.
 */
ctr_object* ctr_array_fill( ctr_object* myself, ctr_argument* argumentList ) {
	size_t n;
	ctr_number fill_times;
	ctr_size i;
	ctr_argument* newArgumentList;
	fill_times = ctr_internal_cast2number( argumentList->object )->value.nvalue;
	if (fill_times < 1) return myself;
	n = (size_t) fill_times;
	newArgumentList = ctr_heap_allocate( sizeof(ctr_argument) );
	newArgumentList->object = argumentList->next->object;
	for(i = 0; i < n; i ++ ) {
		ctr_array_push( myself, newArgumentList );
	}
	ctr_heap_free(newArgumentList);
	return myself;
}

/**
 * @def
 * [ List ] find: [ Object ]
 *
 * @example
 * ☞ x ≔ List ← 1 ; 2 ; 3.
 * ☞ y ≔ x find: 2.
 * ✎ write: y, stop.
 */
ctr_object* ctr_array_index_of( ctr_object* myself, ctr_argument* argumentList ) {
	int found = -1;
	ctr_size i = 0;
	ctr_object* needle = ctr_internal_cast2string(argumentList->object);
	ctr_object* element;
	for(i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
		element = ctr_internal_cast2string( (ctr_object*) *(myself->value.avalue->elements + i) );
		if (
		element->value.svalue->vlen == needle->value.svalue->vlen &&
		strncmp(element->value.svalue->value,needle->value.svalue->value,needle->value.svalue->vlen)==0) {
			found = i;
			break;
		}
	}
	if (found == -1) return ctr_build_nil();
	return ctr_build_number_from_float(found + 1);
}

/**
 * @def
 * Map
 * 
 * @example
 * ☞ x ≔ Map new.
 * x 
 * aaa: ‘a’,
 * bbb: 1.5,
 * ccc: True.
 * ✎ write: x bbb, stop.
 */
ctr_object* ctr_map_new(ctr_object* myclass, ctr_argument* argumentList) {
	ctr_object* s = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	s->link = myclass;
	return s;
}

/**
 * @def
 * [ Map ] type
 *
 * @example
 * ☞ x ≔ Map new.
 * ✎ write: x type, stop.
 */
ctr_object* ctr_map_type(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_string_from_cstring( CTR_DICT_MAP_OBJECT );
}

/**
 * @def
 * [ Map ] put: [ Object ] at: [ Object ]
 *
 * @example
 * ☞ x ≔ Map new.
 * x put: ‘aaa’ at: ‘bbb’.
 * ✎ write: x, stop.
 */
ctr_object* ctr_map_put(ctr_object* myself, ctr_argument* argumentList) {
	char* key;
	long keyLen;
	ctr_object* putKey;
	ctr_object* putValue = argumentList->object;
	ctr_argument* nextArgument = argumentList->next;
	ctr_argument* emptyArgumentList = ctr_heap_allocate(sizeof(ctr_argument));
	emptyArgumentList->next = NULL;
	emptyArgumentList->object = NULL;
	/* Use tostring and not tocode here because tocode will escape quotes, but tostring not, this will preserve the orig key */
	putKey = ctr_send_message(nextArgument->object, CTR_DICT_TOSTRING, strlen(CTR_DICT_TOSTRING), emptyArgumentList);
	/* If developer returns something other than string (ouch, toString), then cast anyway */
	if (putKey->info.type != CTR_OBJECT_TYPE_OTSTRING) {
		putKey = ctr_internal_cast2string(putKey);
	}
	key = ctr_heap_allocate( putKey->value.svalue->vlen * sizeof( char ) );
	keyLen = putKey->value.svalue->vlen;
	memcpy(key, putKey->value.svalue->value, keyLen);
	ctr_internal_object_delete_property(myself, ctr_build_string(key, keyLen), 0);
	ctr_internal_object_add_property(myself, ctr_build_string(key, keyLen), putValue, 0);
	ctr_heap_free( emptyArgumentList );
	ctr_heap_free( key );
	return myself;
}

/**
 * @def
 * [ Map ] [ String ]: [ Object ]
 *
 * @example
 * ☞ x ≔
 * Map new
 * aaa: 11.90,
 * bbb: 12.99,
 * ccc: 13.00.
 * ✎ write: ( x ? ‘aaa’ ), stop.
 * ✎ write: ( x ccc ), stop.
 */
ctr_object* ctr_map_key_value(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* newKey;
	ctr_object* key = ctr_internal_cast2string(argumentList->object);
	newKey = key;
	if (key->value.svalue->vlen>1) {
		newKey = ctr_build_string(key->value.svalue->value,key->value.svalue->vlen-1);
	}
	argumentList->object = argumentList->next->object;
	argumentList->next->object = newKey;
	return ctr_map_put( myself, argumentList );
}

/**
 * @def
 * [ Map ] - [ Object ]
 *
 * @example
 * ☞ x ≔ Map new.
 * x aaa: ‘bbb’, ccc: ‘ddd’.
 * x - ‘ccc’.
 * ✎ write: x, stop.
 */
ctr_object* ctr_map_delete(ctr_object* myself, ctr_argument* argumentList) {
	ctr_internal_object_delete_property(myself, ctr_internal_cast2string(argumentList->object), 0);
	return myself;
}

/**
 * @def
 * [ Map ] entries
 * 
 * @example
 * ☞ x ≔ Map new.
 * x aaa: ‘bbb’, ccc: ‘ddd’.
 * ✎ write: x entries, stop.
 */
ctr_object* ctr_map_keys(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* list;
	ctr_mapitem* m;
	ctr_argument* element;
	list = ctr_array_new( CtrStdArray, argumentList );
	m = myself->properties->head;
	element = ctr_heap_allocate( sizeof( ctr_argument ) );
	while( m ) {
		element->object = m->key;
		ctr_array_push( list, element );
		m = m->next;
	}
	ctr_heap_free( element );
	return list;
}

/**
 * @def
 * [ Map ] values
 * 
 * @example
 * ☞ x ≔ Map new.
 * x aaa: ‘bbb’, ccc: ‘ddd’.
 * ✎ write: x values, stop.
 */
ctr_object* ctr_map_values(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* list;
	ctr_mapitem* m;
	ctr_argument* element;
	list = ctr_array_new( CtrStdArray, argumentList );
	m = myself->properties->head;
	element = ctr_heap_allocate( sizeof( ctr_argument ) );
	while( m ) {
		element->object = m->value;
		ctr_array_push( list, element );
		m = m->next;
	}
	ctr_heap_free( element );
	return list;
}

/**
 * @def
 * [ Map ] at: [ Object ]
 * 
 * @example
 * ☞ x ≔ Map new.
 * x put: ‘a’ at: ‘b’.
 * x put: ‘xxx’ at: ‘yyy’.
 * ✎ write: (x at: ‘b’), stop.
 * ✎ write: (x yyy), stop.
 * ✎ write: (x ? ‘b’), stop.
 */
ctr_object* ctr_map_get(ctr_object* myself, ctr_argument* argumentList) {

	ctr_argument* emptyArgumentList;
	ctr_object*   searchKey;
	ctr_object*   foundObject;

	emptyArgumentList = ctr_heap_allocate(sizeof(ctr_argument));
	emptyArgumentList->next = NULL;
	emptyArgumentList->object = NULL;

	searchKey = argumentList->object;

	/* Give developer a chance to define a key for array */
	searchKey = ctr_send_message(searchKey, CTR_DICT_TOSTRING, strlen(CTR_DICT_TOSTRING), emptyArgumentList);
	ctr_heap_free( emptyArgumentList );

	/* If developer returns something other than string (ouch, toString), then cast anyway */
	if (searchKey->info.type != CTR_OBJECT_TYPE_OTSTRING) {
		searchKey = ctr_internal_cast2string(searchKey);
	}

	foundObject = ctr_internal_object_find_property(myself, searchKey, 0);
	if (foundObject == NULL) foundObject = ctr_build_nil();
	return foundObject;
}


/**
 * @def
 * [ Map ] count
 *
 * @example
 * ☞ x ≔ Map new.
 * x
 * put: ‘a’ at: ‘b’,
 * put: ‘c’ at: ‘d’.
 * ✎ write: x count, stop.
 */
ctr_object* ctr_map_count(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float( myself->properties->size );
}

ctr_object* ctr_map_copy(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* copy = ctr_map_new(CtrStdMap, argumentList);
	ctr_mapitem* m;
	m = myself->properties->head;
	while(m) {
		ctr_argument* arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		ctr_argument* argument2 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		arguments->object = m->value;
		argument2->object = m->key;
		arguments->next = argument2;
		ctr_map_put(copy, arguments);
		m = m->next;
		ctr_heap_free( arguments );
		ctr_heap_free( argument2 );
	}
	return copy;
}

/**
 * @def
 * [ Map ] each: [ Block ]
 *
 * @example
 * (Map new I: 1, II: 2, III: 3) each: { :a :b
 *   ✎ write: a + b, stop.
 * }.
 */
ctr_object* ctr_map_each(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* block = argumentList->object;
	ctr_mapitem* m;
	if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
		CtrStdFlow = ctr_error( CTR_ERR_EXP_BLK, 0 );
	}
	ctr_object* copy = ctr_map_copy(myself, argumentList);
	block->info.sticky = 1;
	m = copy->properties->head;
	while(m && !CtrStdFlow) {
		ctr_argument* arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		ctr_argument* argument2 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		ctr_argument* argument3 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		arguments->object = m->key;
		argument2->object = m->value;
		argument3->object = copy;
		/* keep receiver in block object otherwise, GC will destroy it */
		ctr_gc_internal_pin(block);
		ctr_gc_internal_pin(copy);
		ctr_gc_internal_pin(argument2->object);
		ctr_gc_internal_pin(argument3->object);
		arguments->next = argument2;
		argument2->next = argument3;
		ctr_block_run(block, arguments, NULL);
		if (CtrStdFlow == CtrStdContinue) CtrStdFlow = NULL;
		m = m->next;
		ctr_heap_free( arguments );
		ctr_heap_free( argument2 );
		ctr_heap_free( argument3 );
	}
	if (CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL;
	block->info.mark = 0;
	block->info.sticky = 0;
	return myself;
}


/**
 * @def
 * [ Map ] has: [ Object ]
 *
 * @example
 * ☞ x ≔ Map new.
 * x China: ‘CN’, Russia: ‘RU’.
 * ✎ write: (x has: ‘CN’), stop.
 * ✎ write: (x has: ‘NL’), stop.
 */
ctr_object* ctr_map_has(ctr_object* myself, ctr_argument* argumentList) {
	int found = 0;
	ctr_mapitem* m;
	ctr_object* candidate;
	ctr_object* needle = ctr_internal_cast2string(argumentList->object);
	m = myself->properties->head;
	while(m) {
		candidate = ctr_internal_cast2string(m->value);
		if ( needle->value.svalue->vlen == candidate->value.svalue->vlen ) {
			if ( strncmp(
			candidate->value.svalue->value,
			needle->value.svalue->value, needle->value.svalue->vlen) == 0) {
				found = 1;
			}
		}
		m = m->next;
	}
	return ctr_build_bool(found);
}

/**
 * @def
 * [ Map ] string
 * 
 * @example
 * ☞ x ≔ Map new.
 * x put: ‘a’ at: ‘b’.
 * ☞ y ≔ x string.
 * ✎ write: y, stop.
 */
ctr_object* ctr_map_to_string( ctr_object* myself, ctr_argument* argumentList) {
	ctr_object*  string;
	ctr_mapitem* mapItem;
	ctr_argument* newArgumentList;
	static uint8_t call_depth = 0;
	call_depth++;
	if (call_depth>99) {
		CtrStdFlow = ctr_error( CTR_ERR_NESTING, 0 );
		return ctr_build_empty_string();
	}
	string  = ctr_build_string_from_cstring( CTR_DICT_CODEGEN_MAP_NEW );
	mapItem = myself->properties->head;
	newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	while( mapItem ) {
		newArgumentList->object = ctr_build_string_from_cstring( CTR_DICT_CODEGEN_MAP_PUT );
		ctr_string_append( string, newArgumentList );
		newArgumentList->object = ctr_send_message( mapItem->value, CTR_DICT_CODE, strlen(CTR_DICT_CODE), newArgumentList );
		ctr_string_append( string, newArgumentList );
		newArgumentList->object = ctr_build_string_from_cstring( CTR_DICT_CODEGEN_MAP_PUT_AT );
		ctr_string_append( string, newArgumentList );
		newArgumentList->object = ctr_send_message( mapItem->key, CTR_DICT_CODE, strlen(CTR_DICT_CODE), newArgumentList );
		ctr_string_append( string, newArgumentList );
		mapItem = mapItem->next;
		if ( mapItem ) {
			newArgumentList->object = ctr_build_string_from_cstring( ", " );
			ctr_string_append( string, newArgumentList );
		}
	}
	ctr_heap_free( newArgumentList );
	call_depth--;
	return string;
}
