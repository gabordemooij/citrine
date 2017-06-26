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
 * [Array] new
 *
 * Creates a new Array.
 *
 * Usage:
 *
 * a := Array new.
 *
 * or, the short form:
 *
 * a := Array < 1 ; 2 ; 3.
 *
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
 * [Array] type
 *
 * Returns the string 'Array'.
 *
 **/
ctr_object* ctr_array_type(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_string_from_cstring("Array");
}

/**
 * [Array] push: [Element]
 *
 * Pushes an element on top of the array.
 *
 * Usage:
 *
 * numbers := Array new.
 * numbers push: 3.
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
 * [Array] min
 *
 * Returns the minimum value from an array.
 *
 * Usage:
 *
 * a := Array < 8 ; 4 ; 2 ; 16.
 * m := a min. #2
 *
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
 * [Array] max
 *
 * Returns the maximum value from an array.
 *
 * Usage:
 *
 * a := Array < 8 ; 4 ; 2 ; 16.
 * m := a max. #16
 *
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
 * [Array] sum
 *
 * Takes the sum of an array. This message will calculate the
 * sum of the numerical elements in the array.
 *
 * Usage:
 *
 * a := Array < 1 ; 2 ; 3.
 * s := a sum. #6
 *
 * In the example above, the sum of array will be stored in s and
 * it's value will be 6.
 */
ctr_object* ctr_array_sum(ctr_object* myself, ctr_argument* argumentList) {
	double sum = 0;
	ctr_object* el;
	size_t i = 0;
	for(i = 0; i < myself->value.avalue->head; i++) {
		el = *(myself->value.avalue->elements + i);
		sum += ctr_internal_cast2number(el)->value.nvalue;
	}
	return ctr_build_number_from_float(sum);
}

/**
 * [Array] product
 *
 * Takes the product of an array. On receiving this message, the
 * Array recipient object will calculate the product of its
 * numerical elements.
 *
 * Usage:
 *
 * a := Array < 2 ; 4 ; 8.
 * p := a product. #64
 *
 * In the example above, the product of the array will be calculated
 * because the array receives the message 'product'. The product of the elements
 * ( 2 * 4 * 8 = 64 ) will be stored in p.
 */
ctr_object* ctr_array_product(ctr_object* myself, ctr_argument* argumentList) {
	double product = 1;
	ctr_object* el;
	size_t i = 0;
	for(i = 0; i < myself->value.avalue->head; i++) {
		el = *(myself->value.avalue->elements + i);
		product *= ctr_internal_cast2number(el)->value.nvalue;
	}
	return ctr_build_number_from_float(product);
}

/**
 * [Array] map: [Block].
 *
 * Iterates over the array. Passing each element as a key-value pair to the
 * specified block.
 * The map message will pass the following arguments to the block, the key,
 * the value and a reference to the array itself. The last argument might seem
 * redundant but allows for a more functional programming style.
 *
 * Usage:
 *
 * files map: showName.
 * files map: {
 *   :key :filename :files
 *   Pen write: filename, brk.
 * }.
 */
ctr_object* ctr_array_map(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* block = argumentList->object;
	int i = 0;
	if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
		CtrStdFlow = ctr_build_string_from_cstring("Expected Block.");
		CtrStdFlow->info.sticky = 1;
	}
	block->info.sticky = 1;
	for(i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
		ctr_argument* arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		ctr_argument* argument2 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		ctr_argument* argument3 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		arguments->object = ctr_build_number_from_float((double) i);
		argument2->object = *(myself->value.avalue->elements + i);
		argument3->object = myself;
		arguments->next = argument2;
		argument2->next = argument3;
		ctr_block_run(block, arguments, NULL);
		ctr_heap_free( arguments );
		ctr_heap_free( argument2 );
		ctr_heap_free( argument3 );
		if (CtrStdFlow == CtrStdContinue) CtrStdFlow = NULL;
		if (CtrStdFlow) break;
	}
	if (CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL; /* consume break */
	block->info.mark = 0;
	block->info.sticky = 0;
	return myself;
}

/**
 * [Array] each: [Block].
 *
 *  Alias for [Array] map: [Block].
 */

/**
 * [Array] < [Element1] ; [Element2] ; ...
 *
 * Creates a new instance of an array and initializes this
 * array with a first element, useful for literal-like Array
 * notations.
 *
 * Usage:
 *
 * a := Array < 1 ; 2 ; 3.
 *
 * Note that the ; symbol here is an alias for 'push:'.
 */
ctr_object* ctr_array_new_and_push(ctr_object* myclass, ctr_argument* argumentList) {
	ctr_object* s = ctr_array_new(myclass, NULL);
	return ctr_array_push(s, argumentList);
}

/**
 * [Array] unshift: [Element].
 *
 * Unshift operation for array.
 * Adds the specified element to the beginning of the array.
 *
 * Usage:
 *
 * a := Array new.
 * a push: 1.
 * a unshift: 3. #now contains: 3,1
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
 * [Array] join: [Glue].
 *
 * Joins the elements of an array together in a string
 * separated by a specified glue string.
 *
 * Usage:
 *
 * collection := Array new.
 * collection push: 1, push: 2, push 3.
 * collection join: ','. # results in string: '1,2,3'
 */
ctr_object* ctr_array_join(ctr_object* myself, ctr_argument* argumentList) {
	int i;
	char* result;
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
		if (len == 0) {
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
	if (len > 0) ctr_heap_free( result );
	return resultStr;
}

/**
 * [Array] at: [Index]
 *
 * Returns the element in the array at the specified index.
 * Note that the fisrt index of the array is index 0.
 *
 * Usage:
 *
 * fruits := Array < 'apples' ; 'oranges' ; 'bananas'.
 * fruits at: 1. #returns 'oranges'
 */
ctr_object* ctr_array_get(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* getIndex = argumentList->object;
	int i;
	if (getIndex->info.type != CTR_OBJECT_TYPE_OTNUMBER) {
		printf("Index must be number.\n"); exit(1);
	}
	i = (int) getIndex->value.nvalue;
	if (myself->value.avalue->head <= i || i < myself->value.avalue->tail) {
		CtrStdFlow = ctr_build_string_from_cstring("Index out of bounds.");
		CtrStdFlow->info.sticky = 1;
		return CtrStdNil;
	}
	return *(myself->value.avalue->elements + i);
}

/**
 * [Array] @ [Index]
 *
 * Alias for [Array] at: [Index]
 */

/**
 * [Array] put: [Element] at: [Index]
 *
 * Puts a value in the array at the specified index.
 * Array will be automatically expanded if the index is higher than
 * the maximum index of the array.
 *
 * Usage:
 *
 * fruits := Array new.
 * fruits put: 'apples' at: 5.
 */
ctr_object* ctr_array_put(ctr_object* myself, ctr_argument* argumentList) {

	ctr_object* putValue = argumentList->object;
	ctr_object* putIndex = ctr_internal_cast2number(argumentList->next->object);
	ctr_size putIndexNumber;
	ctr_size head;
	ctr_size tail;

	if (putIndex->value.nvalue < 0) {
		CtrStdFlow = ctr_build_string_from_cstring("Index out of bounds.");
		CtrStdFlow->info.sticky = 1;
		return myself;
	}

	head = (ctr_size) myself->value.avalue->head;
	tail = (ctr_size) myself->value.avalue->tail;
	putIndexNumber = (ctr_size) putIndex->value.nvalue;
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
 * [Array] pop
 *
 * Pops off the last element of the array.
 */
ctr_object* ctr_array_pop(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.avalue->tail >= myself->value.avalue->head) {
		return CtrStdNil;
	}
	myself->value.avalue->head--;
	return *(myself->value.avalue->elements + myself->value.avalue->head);
}

/**
 * [Array] shift
 *
 * Shifts off the first element of the array.
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
 * [Array] count
 *
 * Returns the number of elements in the array.
 */
ctr_object* ctr_array_count(ctr_object* myself, ctr_argument* argumentList) {
	ctr_number d = 0;
	d = (ctr_number) myself->value.avalue->head - myself->value.avalue->tail;
	return ctr_build_number_from_float( (ctr_number) d );
}

/**
 * [Array] from: [Begin] length: [End]
 *
 * Copies part of an array indicated by from and to and
 * returns a new array consisting of a copy of this region.
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
 * [Array] + [Array]
 *
 * Returns a new array, containing elements of itself and the other
 * array.
 */
ctr_object* ctr_array_add(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherArray = argumentList->object;
	ctr_object* newArray = ctr_array_new(CtrStdArray, NULL);
	int i;
	for(i = myself->value.avalue->tail; i<myself->value.avalue->head; i++) {
		ctr_argument* pushArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		ctr_argument* elnumArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		ctr_object* elnum = ctr_build_number_from_float((ctr_number) i);
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
			ctr_object* elnum = ctr_build_number_from_float((ctr_number) i);
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
 * @internal
 *
 * Internal sort function, for use with ArraySort.
 * Interfaces with qsort-compatible function.
 */
ctr_object* temp_sorter;
int ctr_sort_cmp(const void * a, const void * b) {
	ctr_argument* arg1 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	ctr_argument* arg2 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	ctr_object* result;
	ctr_object* numResult;
	arg1->next = arg2;
	arg1->object = *((ctr_object**) a);
	arg2->object = *((ctr_object**) b);
	result = ctr_block_run(temp_sorter, arg1, NULL);
	numResult = ctr_internal_cast2number(result);
	ctr_heap_free( arg1 );
	ctr_heap_free( arg2 );
	return (int) numResult->value.nvalue;
}

/**
 * [Array] sort: [Block]
 *
 * Sorts the contents of an array using a sort block.
 * Uses qsort.
 */
ctr_object* ctr_array_sort(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* sorter = argumentList->object;
	if (sorter->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
		CtrStdFlow = ctr_build_string_from_cstring("Expected block.");
		CtrStdFlow->info.sticky = 1;
		return myself;
	}
	temp_sorter = sorter;
	qsort(myself->value.avalue->elements, myself->value.avalue->head, sizeof(ctr_object*), ctr_sort_cmp);
	return myself;
}

/**
 * [Array] toString
 * 
 * Returns a string representation of the array and its contents.
 * This representation will be encoded in the Citrine language itself and is
 * therefore evallable.
 * 
 * Usage:
 * 
 * a := Array < 'hello' ; 'world'.
 * b := a toString.
 * c := b eval.
 * x := c @ 1. #world
 * 
 * toString messages are implicitly send by some objects, for instance when
 * attempting to write an Array using a Pen.
 */
ctr_object* ctr_array_to_string( ctr_object* myself, ctr_argument* argumentList ) {
	int i;
	ctr_object* arrayElement;
	ctr_argument* newArgumentList;
	ctr_object* nest;
	ctr_object* string = ctr_build_empty_string();
	newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	if ( myself->value.avalue->tail == myself->value.avalue->head ) {
		newArgumentList->object = ctr_build_string_from_cstring("Array new ");
		string = ctr_string_append( string, newArgumentList );
	} else {
		newArgumentList->object = ctr_build_string_from_cstring("Array < ");
		string = ctr_string_append( string, newArgumentList );
	}
	for(i=myself->value.avalue->tail; i<myself->value.avalue->head; i++) {
		arrayElement = *( myself->value.avalue->elements + i );
		if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTBOOL || arrayElement->info.type == CTR_OBJECT_TYPE_OTNUMBER ) {
			newArgumentList->object = arrayElement;
			string = ctr_string_append( string, newArgumentList );
		}
		if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTSTRING ) {
			newArgumentList->object = ctr_build_string_from_cstring("'");
			string = ctr_string_append( string, newArgumentList );
			newArgumentList->object = arrayElement;
			string = ctr_string_append( string, newArgumentList );
			newArgumentList->object = ctr_build_string_from_cstring("'");
			string = ctr_string_append( string, newArgumentList );
		}
		if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTARRAY ) {
			newArgumentList->object = ctr_build_string_from_cstring("(");
			ctr_string_append( string, newArgumentList );
			newArgumentList->object = arrayElement;
			nest = ctr_array_to_string( arrayElement, newArgumentList );
			newArgumentList->object = nest;
			string = ctr_string_append( string, newArgumentList );
			newArgumentList->object = ctr_build_string_from_cstring(")");
			ctr_string_append( string, newArgumentList );
		}
		if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTOBJECT ) {
			ctr_object* link;
			ctr_object* testObject = arrayElement;
			int isMap = 0;
			while( ( link = testObject->link ) ) {
				if ( link == CtrStdMap ) {
					isMap = 1;
					break;
				}
			}
			if ( isMap ) {
				newArgumentList->object = ctr_build_string_from_cstring("(");
				ctr_string_append( string, newArgumentList );
				newArgumentList->object = arrayElement;
				nest = ctr_map_to_string( arrayElement, newArgumentList );
				newArgumentList->object = nest;
				string = ctr_string_append( string, newArgumentList );
				newArgumentList->object = ctr_build_string_from_cstring(")");
				ctr_string_append( string, newArgumentList );
			}
		}
		if (  (i + 1 )<myself->value.avalue->head ) {
			newArgumentList->object = ctr_build_string_from_cstring(" ; ");
			string = ctr_string_append( string, newArgumentList );
		}
	}
	ctr_heap_free( newArgumentList );
	return string;
}


/**
 * Map
 *
 * Creates a Map object.
 *
 * Usage:
 *
 * files := Map new.
 * files put: 'readme.txt' at: 'textfile'.
 */
ctr_object* ctr_map_new(ctr_object* myclass, ctr_argument* argumentList) {
	ctr_object* s = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	s->link = myclass;
	return s;
}

/**
 * [Map] type
 *
 * Returns the string 'Map'.
 *
 **/
ctr_object* ctr_map_type(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_string_from_cstring("Map");
}

/**
 * [Map] put: [Element] at: [Key]
 *
 * Puts a key-value pair in a map.
 *
 * Usage:
 *
 * map put: 'hello' at: 'world'.
 *
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

	putKey = ctr_send_message(nextArgument->object, "toString", 8, emptyArgumentList);

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
 * [Map] at: [Key]
 *
 * Retrieves the value specified by the key from the map.
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
	searchKey = ctr_send_message(searchKey, "toString", 8, emptyArgumentList);
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
 * [Map] @ [Key]
 *
 * Alias for [Map] at: [Key].
 *
 */

/**
 * [Map] count
 *
 * Returns the number of elements in the map.
 */
ctr_object* ctr_map_count(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float( myself->properties->size );
}

/**
 * [Map] each: [Block]
 *
 * Iterates over the map, passing key-value pairs to the specified block.
 * Note that within an each/map block, 'me' and 'my' refer to the collection.
 */
ctr_object* ctr_map_each(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* block = argumentList->object;
	ctr_mapitem* m;
	if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
		CtrStdFlow = ctr_build_string_from_cstring("Expected Block.");
		CtrStdFlow->info.sticky = 1;
	}
	block->info.sticky = 1;
	m = myself->properties->head;
	while(m && !CtrStdFlow) {
		ctr_argument* arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		ctr_argument* argument2 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		ctr_argument* argument3 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		arguments->object = m->key;
		argument2->object = m->value;
		argument3->object = myself;
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
 * [Map] toString
 *
 * Returns a string representation of a map encoded in Citrine itself.
 * This will give you an
 * evallable representation of the map and all of its members.
 *
 * Usage
 *
 * m := (Map new) put: 'hello' at: 'world'.
 * x := m toString
 * m := x eval.
 *
 * The toString method is automatically invoked when attempting to
 * print a Map:
 *
 * Pen write: (Map new). #prints Map new.
 *
 */
ctr_object* ctr_map_to_string( ctr_object* myself, ctr_argument* argumentList) {
	
	ctr_object*  string;
	ctr_mapitem* mapItem;
	ctr_argument* newArgumentList;
	
	string  = ctr_build_string_from_cstring( "(Map new) " );
	mapItem = myself->properties->head;
	newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	
	while( mapItem ) {
		newArgumentList->object = ctr_build_string_from_cstring( "put:" );
		ctr_string_append( string, newArgumentList );
		if ( mapItem->value->info.type == CTR_OBJECT_TYPE_OTBOOL || mapItem->value->info.type == CTR_OBJECT_TYPE_OTNUMBER ) {
			newArgumentList->object = mapItem->value;
			ctr_string_append( string, newArgumentList );
		} else if ( mapItem->value->info.type == CTR_OBJECT_TYPE_OTSTRING ) {
			newArgumentList->object = ctr_build_string_from_cstring( "'" );
			ctr_string_append( string, newArgumentList );
			newArgumentList->object = mapItem->value;
			ctr_string_append( string, newArgumentList );
			newArgumentList->object = ctr_build_string_from_cstring( "'" );
			ctr_string_append( string, newArgumentList );
		} else {
			newArgumentList->object = ctr_build_string_from_cstring( "(" );
			ctr_string_append( string, newArgumentList );
			newArgumentList->object = mapItem->value;
			ctr_string_append( string, newArgumentList );
			newArgumentList->object = ctr_build_string_from_cstring( ")" );
			ctr_string_append( string, newArgumentList );
		}
		newArgumentList->object = ctr_build_string_from_cstring( " at:" );
		ctr_string_append( string, newArgumentList );
		if ( mapItem->key->info.type == CTR_OBJECT_TYPE_OTBOOL || mapItem->key->info.type == CTR_OBJECT_TYPE_OTNUMBER ) {
			newArgumentList->object = mapItem->key;
			ctr_string_append( string, newArgumentList );
		} else if ( mapItem->key->info.type == CTR_OBJECT_TYPE_OTSTRING ) {
			newArgumentList->object = ctr_build_string_from_cstring( "'" );
			ctr_string_append( string, newArgumentList );
			newArgumentList->object = mapItem->key;
			ctr_string_append( string, newArgumentList );
			newArgumentList->object = ctr_build_string_from_cstring( "'" );
			ctr_string_append( string, newArgumentList );
		} else {
			newArgumentList->object = ctr_build_string_from_cstring( "(" );
			ctr_string_append( string, newArgumentList );
			newArgumentList->object = mapItem->key;
			ctr_string_append( string, newArgumentList );
			newArgumentList->object = ctr_build_string_from_cstring( ")" );
			ctr_string_append( string, newArgumentList );
		}
		mapItem = mapItem->next;
		if ( mapItem ) {
			newArgumentList->object = ctr_build_string_from_cstring( ", " );
			ctr_string_append( string, newArgumentList );
		}
	}
	ctr_heap_free( newArgumentList );
	return string;
	
}

