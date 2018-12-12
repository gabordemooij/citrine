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
 * [List] new
 *
 * Creates a new list or array. List is an alias for array.
 * An array is a collection of items. To create a new array,
 * send the 'new' message to the array. To add an element send
 * the 'push:' message to an array with the element to add as
 * an argument. Instead of using the push-message you can also
 * use the • message. This message is suitable for vertically
 * written arrays because they look similar to lists seen in
 * regular documents. Besides 'push:' and • you can also use
 * the ; message to push an new element on top of the array.
 * The arrow message is the same as 'new' plus 'push:', just a
 * shorter notation. The ; message is very suitable for
 * horizontally written arrays. Finally, the last example
 * depicts a notation using just ascii characters.
 *
 * Usage:
 *
 * ☞ meals :=
 *	List new
 *	• 'hamburger'
 *	• 'pizza'
 *	• 'haggis'.
 *
 * ☞ todo := List ← 'dishes' ; 'cleaning'.
 * 
 * In other languages:
 * Dutch: Reeks nieuw. Maakt een nieuwe reeks.
 * Gebruik ← om een lijst te maken en direct te vullen.
 * Voorbeeld: ☞ oneven := Reeks ← 1 ; 3 ; 5.
 */

/**
 * [List] new
 *
 * Creates a new array.
 * An array is a collection of items. To create a new array,
 * send the 'new' message to the array. To add an element send
 * the 'push:' message to an array with the element to add as
 * an argument. Instead of using the push-message you can also
 * use the • message. This message is suitable for vertically
 * written arrays because they look similar to lists seen in
 * regular documents. Besides 'push:' and • you can also use
 * the ; message to push an new element on top of the array.
 * The arrow message is the same as 'new' plus 'push:', just a
 * shorter notation. The ; message is very suitable for
 * horizontally written arrays. Finally, the last example
 * depicts a notation using just ascii characters.
 *
 * Usage:
 *
 * ☞ meals :=
 *	List new
 *	• 'hamburger'
 *	• 'pizza'
 *	• 'haggis'.
 *
 * ☞ todo := List ← 'dishes' ; 'cleaning'.
 * 
 * In other languages:
 * Dutch: Reeks nieuw. Maakt een nieuwe reeks.
 * Gebruik ← om een lijst te maken en direct te vullen.
 * Voorbeeld: ☞ oneven := Reeks ← 1 ; 3 ; 5.
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
 * [List] type
 *
 * Returns the string description for this object type.
 * 
 * In other languages:
 * Dutch: [Reeks] type. | Geeft een beschrijving van het object.
 *
 **/
ctr_object* ctr_array_type(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_string_from_cstring( CTR_DICT_LIST_OBJECT );
}

/**
 * [List] append: [Element]
 *
 * Adds an element to the end of the list.
 * You can also use add: to do this or one of the symbolic
 * representations: • and ;. Depending on the context, one might be
 * more readable than the other.
 *
 * Usage:
 *
 * numbers := List new.
 * numbers append: 3.
 * numbers ; 3.
 * numbers • 3.
 * 
 * In other languages:
 * Dutch: [Reeks] toevoegen: [Object]. | Voegt iets toe aan een reeks.
 * Alternatieve notaties: • of ;.
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
 * [List] minimum
 *
 * Returns the minimum value in a list.
 * In the example this message will return the number 2.
 * 
 * Usage:
 *
 * a := List ← 8 ; 4 ; 2 ; 16.
 * m := a minimum.
 * 
 * In other languages:
 * Dutch: [Reeks] maximum | Geeft de laagste waarde uit de reeks terug.
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
 * [List] maximum
 *
 * Returns the maximum value in a list.
 * In the example this will yield the number 16.
 *
 * Usage:
 *
 * a := List ← 8 ; 4 ; 2 ; 16.
 * m := a maximum.
 * 
 * In other languages:
 * Dutch: [Reeks] maximum | Geeft de hoogste waarde uit de reeks terug.
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
 * [List] map: [Block].
 *
 * Iterates over the array. Passing each element as a key-value pair to the
 * specified block.
 * The map message will pass the following arguments to the block, the key,
 * the value and a reference to the array itself. The last argument might seem
 * redundant but allows for a more functional programming style. Instead of map,
 * you can also use each:.
 *
 * Usage:
 *
 * files map: showName.
 * files map: {
 *   :key :filename :files
 *   ✎ write: filename.
 * }.
 * 
 * files each: {
 *   :key :filename :files
 *   ✎ write: filename.
 * }.
 * 
 * In other languages:
 * Dutch: [Reeks] lijst: [Codeblok] | Maakt van een reeks 
 */
ctr_object* ctr_array_map(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* block = argumentList->object;
	int i = 0;
	if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
		CtrStdFlow = ctr_build_string_from_cstring( CTR_ERR_EXP_BLK );
		CtrStdFlow->info.sticky = 1;
	}
	for(i = myself->value.avalue->tail; i < myself->value.avalue->head; i++) {
		ctr_argument* arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		ctr_argument* argument2 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		ctr_argument* argument3 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		arguments->object = ctr_build_number_from_float((double) i);
		argument2->object = *(myself->value.avalue->elements + i);
		argument3->object = myself;
		arguments->next = argument2;
		argument2->next = argument3;
		/* keep receiver in block object otherwise, GC will destroy it */
		ctr_gc_internal_pin(block);
		ctr_gc_internal_pin(myself);
		ctr_gc_internal_pin(argument2->object);
		ctr_block_run(block, arguments, myself);
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
 * [List] ← [Element1] ; [Element2] ; ...
 *
 * Creates a new instance of a list and initializes this
 * array with a first element, useful for literal-like List
 * notations. In the example we create a new list consisting
 * of the numbers 1, 2 and 3.
 *
 * Usage:
 *
 * a := List ← 1 ; 2 ; 3.
 *
 * In other languages:
 * Dutch: [Reeks] ← [Element1] ; [Element2] ; ... | De pijl maakt een nieuwe reeks en voegt het eerste
 * element direct toe, opvolgende elementen kunnen worden gescheiden door puntkomma's (;), hiermee
 * voegt men telkens een nieuw element toe aan de reeks.
 */
ctr_object* ctr_array_new_and_push(ctr_object* myclass, ctr_argument* argumentList) {
	ctr_object* s = ctr_array_new(myclass, NULL);
	return ctr_array_push(s, argumentList);
}

/**
 * [List] prepend: [Element].
 *
 * Adds the specified element to the beginning of the array.
 * At the end of the example code, the list will consist of the
 * numbers: 3 and 1 (in that order).
 *
 * Usage:
 *
 * a := List new.
 * a append: 1.
 * a prepend: 3.
 *
 * In other languages:
 * Dutch: [Reeks] invoegen: [Object] | Voegt het aangegeven object in aan het begin van de reeks.
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
 * [List] join: [String].
 *
 * Joins the elements of a list together in a string
 * separated by a specified glue string. The example
 * code results in the string: '1,2,3'.
 *
 * Usage:
 *
 * collection := List new.
 * collection append: 1, append: 2, append 3.
 * collection join: ','.
 *
 * In other languages:
 * Dutch: [Reeks] samenvoegen: [Tekst] | Maakt een tekst door
 * reekselementen samen te voegen met gespecificeerde koppelteken(s).
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
 * [List] position: [Number]
 *
 * Returns the element in the list at the specified position.
 * Note that the first position of the list is index 0.
 * If you attempt to retrieve an element of the list
 * using a an index that is something other than a number
 * a catchable error will be triggered. An error will
 * also be triggered if your position is out of bounds
 * (i.e. outside the list). Instead of the message
 * 'position:' you can also send the message '?'.
 *
 * Usage:
 *
 * ☞ fruits  := List ← 'apples' ; 'oranges' ; 'bananas'.
 * ☞ oranges := fruits position: 1.
 * ☞ oranges := fruits ? 1.
 *
 * In other languages:
 * Dutch: [Reeks] positie: [Getal] | Geeft het object op de aangegeven plek in de reeks.
 */
ctr_object* ctr_array_get(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* getIndex = argumentList->object;
	int i;
	if (getIndex->info.type != CTR_OBJECT_TYPE_OTNUMBER) {
		CtrStdFlow = ctr_build_string_from_cstring( CTR_ERR_EXP_NUM );
		CtrStdFlow->info.sticky = 1;
		return CtrStdNil;
	}
	i = (int) getIndex->value.nvalue;
	if (myself->value.avalue->head <= (i + myself->value.avalue->tail) || i < 0) {
		CtrStdFlow = ctr_build_string_from_cstring( CTR_ERR_BOUNDS );
		CtrStdFlow->info.sticky = 1;
		return CtrStdNil;
	}
	ctr_object* q =  *(myself->value.avalue->elements + myself->value.avalue->tail + i);
	return q;
}

/**
 * [List] first.
 * 
 * Returns the first element of the list.
 * If the list is empty, Nil will be returned.
 *
 * In other languages:
 * Dutch: [Reeks] eerste | Geef het eerste element uit de reeks.
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
 * [List] last.
 * 
 * Returns the last element of the list.
 * If the list is empty, Nil will be returned.
 *
 * In other languages:
 * Dutch: [Reeks] laatste | Geeft het laatste element uit de reeks.
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
 * [List] second last.
 * 
 * Returns the second last element of the list.
 * If the list is empty, Nil will be returned.
 *
 * In other languages:
 * Dutch: [Reeks] op-een-na-laatste | Geeft het op-een-na-laatste element uit reeks.
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
 * [List] put: [Object] at: [Number]
 *
 * Puts an object (which can be anything) in the list
 * at the specified position.
 * The list will be automatically expanded if the position number
 * is higher than the maximum of the list.
 *
 * Usage:
 *
 * ☞ fruits := List new.
 * ☞ fruits put: 'apples' at: 5.
 *
 * In other languages:
 * Dutch: [Reeks] zet: [Object] bij: [Getal]
 * Plaatst het object op de aangegeven positie in de reeks.
 */
ctr_object* ctr_array_put(ctr_object* myself, ctr_argument* argumentList) {

	ctr_object* putValue = argumentList->object;
	ctr_object* putIndex = ctr_internal_cast2number(argumentList->next->object);
	ctr_size putIndexNumber;
	ctr_size head;
	ctr_size tail;

	if (putIndex->value.nvalue < 0) {
		CtrStdFlow = ctr_build_string_from_cstring( CTR_ERR_BOUNDS );
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
 * [List] pop
 *
 * Pops off the last element of the array.
 *
 * In other languages:
 * Dutch: [Reeks] eraf | Schuift laatste element uit reeks af en geeft het terug.
 */
ctr_object* ctr_array_pop(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.avalue->tail >= myself->value.avalue->head) {
		return CtrStdNil;
	}
	myself->value.avalue->head--;
	return *(myself->value.avalue->elements + myself->value.avalue->head);
}

/**
 * [List] - [Number]
 *
 * Deletes the element specified by the index number and
 * shrinks the list accordingly. If the position number does not exist,
 * the list will remain the same. This operation changes the list itself.
 * The example will remove element 1 (2) from the list.
 *
 * Usage:
 *
 * ☞ x := List ← 1 ; 2 ; 3.
 * ☞ x - 1.
 *
 * In other languages:
 * Dutch: [Reeks] - [Getal] | Verwijder het element op de aangegeven plek.
 */
ctr_object* ctr_array_delete(ctr_object* myself, ctr_argument* argumentList) {
	ctr_size index = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	ctr_size length = (ctr_size) myself->value.avalue->head - myself->value.avalue->tail;
	ctr_size i;
	ctr_size found = 0;
	for( i = index; i <  length-1; i ++ ) {
		*(myself->value.avalue->elements + i) = *(myself->value.avalue->elements + (i+1));
		found = 1;
	}
	if (found) {
		myself->value.avalue->head--;
	}
	return myself;
}

/**
 * [List] shift
 *
 * Shifts off the first element of the list.
 *
 * In other languages:
 * Dutch: [Reeks] afschuiven | Schuift het eerste element van de reeks.
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
 * [List] count
 *
 * Returns the number of elements in the list.
 *
 * In other languages:
 * Dutch: [Reeks] aantal | Geeft het aantal elementen in de reeks.
 */
ctr_object* ctr_array_count(ctr_object* myself, ctr_argument* argumentList) {
	ctr_number d = 0;
	d = (ctr_number) myself->value.avalue->head - myself->value.avalue->tail;
	return ctr_build_number_from_float( (ctr_number) d );
}

/**
 * [List] from: [Begin] length: [End]
 *
 * Copies part of an array indicated by from and to and
 * returns a new array consisting of a copy of this region.
 *
 * In other languages:
 * Dutch: [Reeks] van: [Getal] lengte: [Getal] | Geeft subreeks.
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
 * [List] replace: [Number] length: [Number] with: [List].
 *
 * Returns a copy of the list with the specified elements replaced.
 * The first argument indicates the start index to begin the replacement.
 * Here, 0 means the beginning of the list.
 * The second argument (length)
 * must indicate the number of elements to delete in the copy, counting
 * from the starting point. Finally, one has to provide the replacement
 * list as the third argument.
 * If the replacement list is empty, the specified elements will only be
 * removed from the copy.
 * If the replacement is not an array an error will be thrown.
 *
 * Usage:
 *
 * ☞ buy := cakes
 *     replace: 1
 *     length: 2
 *     with: ( List ← 'cinnamon' ; 'pineapple' ).
 *
 * In other languages:
 * Dutch: [Reeks] vervang: [Getal] lengte: [Getal] door: [Reeks]
 * Vervangt een deel van de reeks door een andere reeks.
 */
ctr_object* ctr_array_splice(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* newArray = ctr_array_new(CtrStdArray, NULL);
	ctr_object* start = ctr_internal_cast2number(argumentList->object);
	ctr_object* deleteCount = ctr_internal_cast2number(argumentList->next->object);
	ctr_object* replacement = argumentList->next->next->object;
	ctr_object* remainder;
	ctr_argument* sliceFromArg;
	ctr_argument* sliceLengthArg;
	ctr_argument* replacementArg;
	ctr_argument* remainderArg;
	ctr_size n;
	if ( replacement->info.type != CTR_OBJECT_TYPE_OTARRAY ) {
		CtrStdFlow = ctr_error_text( CTR_ERR_EXP_ARR );
		return myself;
	}
	n = ( start->value.nvalue + deleteCount->value.nvalue );
	sliceFromArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	sliceLengthArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	replacementArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	remainderArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	sliceFromArg->object = ctr_build_number_from_float(0);
	sliceLengthArg->object = start;
	sliceFromArg->next = sliceLengthArg;
	newArray = ctr_array_from_length( myself, sliceFromArg );
	replacementArg->object = replacement;
	newArray = ctr_array_add(newArray, replacementArg);
	sliceFromArg->object = ctr_build_number_from_float( n );
	if ( n < (myself->value.avalue->head - myself->value.avalue->tail) ) {
		sliceLengthArg->object = ctr_build_number_from_float( (myself->value.avalue->head - myself->value.avalue->tail) - n );
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
 * [List] + [List]
 *
 * Returns a new list, containing elements of itself and the other
 * list.
 *
 * In other languages:
 * Dutch: [Reeks] + [Reeks] | Geeft de reeks die bestaat uit de samenvoeging van gegeven reeksen.
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
 * [List] by: [List].
 *
 * Combines the first list with the second one, thus creating
 * a map. The keys of the newly generated map will be provided by the
 * first list while the values are extracted from the second one.
 * In the example we derive a temperature map from a pair of lists
 * (cities and temperatures).
 *
 * Usage:
 *
 * ☞ city        := List ← 'London' ; 'Paris' ; 'Berlin'.
 * ☞ temperature := List ← '15' ; '16' ; '15'.
 * ☞ weather := temperature by: city.
 *
 * In other languages:
 * Dutch: [Reeks] per: [Reeks]
 * Maakt een Lijst door elementen uit de eerste reeks te koppelen
 * aan de elementen op dezelfde plek uit de tweede reeks.
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
			index->object = ctr_build_number_from_float((ctr_number) i);
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
 * [List] copy
 *
 * Copies the list. The list object will answer this message by
 * returning a shallow copy of itself. This means that the values in the
 * newly returned list can be replaced or deleted without affecting
 * the original one. However, modifying the values in the list will
 * still cause their counterparts in the original list to be modified
 * as well.
 * In the example we replace the first item (1) in b with 999.
 * The first element in a will still be 1 though because we have created
 * copy b by sending the message 'copy' to a and assiging the result
 * to b.
 *
 * Usage:
 *
 * ☞ a := List ← 1 ; 2 ; 3.
 * ☞ b := a copy.
 * b put: 999 at: 1.
 *
 * In other languages:
 * Dutch: [Reeks] kopieer | Maakt een kopie van de reeks.
 */
ctr_object* ctr_array_copy(ctr_object* myself, ctr_argument* argumentList) {
	ctr_size i = 0;
	ctr_object* copy = ctr_array_new( CtrStdArray, argumentList );
	ctr_argument* arg = ctr_heap_allocate(sizeof(ctr_argument));
	ctr_argument* index   = ctr_heap_allocate( sizeof( ctr_argument ) );
	for(i = myself->value.avalue->tail; i<myself->value.avalue->head; i++) {
		index->object = ctr_build_number_from_float((ctr_number) i);
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
	result = ctr_block_run(temp_sorter, arg1, temp_self);
	numResult = ctr_internal_cast2number(result);
	ctr_heap_free( arg1 );
	ctr_heap_free( arg2 );
	return (int) numResult->value.nvalue;
}

/**
 * [List] sort: [Block]
 *
 * Sorts the contents of an list using a sort block.
 * Uses qsort.
 *
 * In other languages:
 * Dutch: [Reeks] sorteer: [Codeblok]
 * Sorteert de reeks door de elementen door het codeblok te halen.
 */
ctr_object* ctr_array_sort(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* sorter = argumentList->object;
	if (sorter->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
		CtrStdFlow = ctr_build_string_from_cstring( CTR_ERR_EXP_BLK );
		CtrStdFlow->info.sticky = 1;
		return myself;
	}
	temp_sorter = sorter;
	temp_self = myself;
	qsort((myself->value.avalue->elements+myself->value.avalue->tail), myself->value.avalue->head-myself->value.avalue->tail, sizeof(ctr_object*), ctr_sort_cmp);
	return myself;
}

/**
 * [List] string
 * 
 * Returns a string representation of the list and its contents.
 * This representation will be encoded in the Citrine language itself and can
 * therefore be evaluated again.
 * In the example: 'string' messages are implicitly
 * send by some objects, for instance when
 * attempting to write a List using a Pen.
 *
 * Usage:
 * 
 * ☞ a := List ← 'hello' ; 'world'.
 * ☞ b := a string.
 * ☞ c := b evaluate.
 *
 * In other languages:
 * Dutch: [Reeks] tekst
 * Geeft een tekstuele versie van de reeks terug. Deze tekst kan opnieuw worden
 * ingelezen door Citrine om er een reeks van te maken (evalueer).
 */
ctr_object* ctr_array_to_string( ctr_object* myself, ctr_argument* argumentList ) {
	int i;
	ctr_object* arrayElement;
	ctr_argument* newArgumentList;
	ctr_object* string = ctr_build_empty_string();
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
		if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTBOOL || arrayElement->info.type == CTR_OBJECT_TYPE_OTNUMBER
		|| arrayElement->info.type == CTR_OBJECT_TYPE_OTNIL ) {
			newArgumentList->object = arrayElement;
			string = ctr_string_append( string, newArgumentList );
		} else if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTSTRING ) {
			newArgumentList->object = ctr_build_string_from_cstring("'");
			string = ctr_string_append( string, newArgumentList );
			newArgumentList->object = ctr_string_quotes_escape( arrayElement, newArgumentList );
			string = ctr_string_append( string, newArgumentList );
			newArgumentList->object = ctr_build_string_from_cstring("'");
			string = ctr_string_append( string, newArgumentList );
		} else {
			newArgumentList->object = ctr_build_string_from_cstring("(");
			ctr_string_append( string, newArgumentList );
			newArgumentList->object = arrayElement;
			string = ctr_string_append( string, newArgumentList );
			newArgumentList->object = ctr_build_string_from_cstring(")");
			ctr_string_append( string, newArgumentList );
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
 * [List] fill: [Number] with: [Object]
 *
 * Fills the list with the specified number of objects.
 *
 * Usage:
 *
 * ☞ a := List new fill: 42 with: 'x'.
 *
 * In other languages:
 * Dutch: [Reeks] vul: [Getal] met: [Object]
 * Vult de reeks op met een gespecificeerd aantal elementen.
 */
ctr_object* ctr_array_fill( ctr_object* myself, ctr_argument* argumentList ) {
	size_t n;
	int i;
	ctr_argument* newArgumentList;
	n = ctr_internal_cast2number( argumentList->object )->value.nvalue;
	newArgumentList = ctr_heap_allocate( sizeof(ctr_argument) );
	newArgumentList->object = argumentList->next->object;
	for(i = 0; i < n; i ++ ) {
		ctr_array_push( myself, newArgumentList );
	}
	ctr_heap_free(newArgumentList);
	return myself;
}

/**
 * [List] find: [Object].
 *
 * Checks whether the specified object occurs in the list
 * and returns the index number if so.
 * If not, the index number -1 will be returned. Note that the comparison
 * will be performed by converting both values to strings.
 *
 * In other languages:
 * Dutch: [Reeks] vind: [Object]
 * Geeft de positie van het object terug of -1 als niet gevonden.
 */
ctr_object* ctr_array_index_of( ctr_object* myself, ctr_argument* argumentList ) {
	int64_t found = -1, i = 0;
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
	return ctr_build_number_from_float(found);
}

/**
 * Map
 *
 * Creates a Map object.
 *
 * Usage:
 *
 * ☞ files := Map new.
 * ☞ files put: 'readme.txt' at: 'textfile'.
 *
 * In other languages:
 * Dutch: Lijst een verzameling van gepaarde elementen in de vorm sleutel = waarde.
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
 * In other languages:
 * Dutch: [Lijst] tekst | Geeft een tekstuele beschrijving van de Lijst ('Lijst').
 */
ctr_object* ctr_map_type(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_string_from_cstring( CTR_DICT_MAP_OBJECT );
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
 * In other languages:
 * Dutch: [Lijst] zet: [Object] bij: [Object]
 * Zet het gespecificeerde object element bij de plek die bekend staat als
 * het andere object. Net als bij een reeks, alleen in dit geval is het tweede
 * Object de sleutel waarmee het eerste object weer uit de lijst gevist kan
 * worden.
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
 * [Map] [Key]: [Value]
 *
 * You can fill the map object with key-value pairs by sending any
 * binary or keyword message that is not part if its standard behaviour.
 * Likewise you can retrieve any value from the map by sending the corresponding key
 * as a unary message. This allows for a very natural looking notation to create
 * and modify map objects.
 *
 * Usage:
 *
 * ☞ menu := Map new
 *	Margherita: 11.90,
 *	Hawaii: 12.99,
 *	QuattroFormaggi: 13.00.
 *
 * ✎ write: ( menu ? 'Hawaii' ), brk. 
 * ✎ write: ( menu Margherita ), brk.
 *
 * In other languages:
 *
 * Dutch: [Lijst] [Object]: [Object]
 * Snelle en leesbare notatie om objecten toe te voegen aan een lijst.
 * Elk bericht dat niet wordt herkend wordt door de lijst als een
 * sleutel beschouwd, het opvolgende object zal op de plek worden
 * gezet in de lijst die door de sleutel wordt aangegeven. Dus om een
 * menukaart te vullen kan men zeggen:
 *
 * ☞ menu := Lijst nieuw.
 * menu pannekoek: 10. (hier kopppelen we pannekoek aan 10)
 *
 * Om nu op te vragen hoeveel een pannekoek kost schrijven we:
 * ☞ prijs := menu pannekoek.
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
 * [Map] - [String]
 *
 * Deletes the entry, identified by the key specified in [String], from
 * the map.
 *
 * In other languages:
 * Dutch: [Lijst] - [Tekst] | Verwijderd de ingang voor aangegeven sleutel.
 */
ctr_object* ctr_map_delete(ctr_object* myself, ctr_argument* argumentList) {
	ctr_internal_object_delete_property(myself, ctr_internal_cast2string(argumentList->object), 0);
	return myself;
}

/**
 * [Map] keys
 *
 * Returns an array containing all the keys in the map.
 * The order of the keys is undefined. Use the sort message
 * to enforce a specific order. The 'entries' message
 * does exactly the same and is an alias for 'keys'.
 *
 * Usage:
 *
 * ☞ city        := List ← 'London' ; 'Paris' ; 'Berlin'.
 * ☞ temperature := List ← '15' ; '16' ; '15'.
 *
 * ☞ weather := temperature by: city.
 * cities := weather entries sort: {
 * 	:a :b  ↲ (a compare: b).
 * }.
 *
 * In other languages:
 * Dutch: [Lijst] ingangen | Geeft alle sleutels uit de lijst als een reeks.
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
 * [Map] values
 *
 * Returns an array containing all the keys in the map.
 * The order of the keys is undefined. Use the sort message
 * to enforce a specific order.
 *
 * Usage:
 *
 * ☞ city        := List ← 'London' ; 'Paris' ; 'Berlin'.
 * ☞ temperature := List ← '15' ; '16' ; '15'.
 *
 * ☞ weather := temperature by: city.
 * temperatures := weather values sort: {
 * 	:a :b  ↲ (a compare: b).
 * }.
 *
 * In other languages:
 * Dutch: [Lijst] waarden | Geeft alle waarden uit de lijst als een reeks.
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
 * [Map] at: [Key]
 *
 * Retrieves the value specified by the key from the map.
 *
 * In other languages:
 * Dutch: [Lijst] bij: [Object] | Geeft de waarde bij de bijbehorende sleutel.
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
 * [Map] ? [Key]
 *
 * Alias for [Map] at: [Key].
 *
 * In other languages:
 * Dutch: [Lijst] ? [Object] | Geeft de waarde bij de sleutel.
 */

/**
 * [Map] count
 *
 * Returns the number of elements in the map.
 *
 * In other languages:
 * Dutch: [Lijst] aantal | Geeft het aantal zaken op de lijst.
 */
ctr_object* ctr_map_count(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float( myself->properties->size );
}

/**
 * [Map] each: [Block]
 *
 * Iterates over the map, passing key-value pairs to the specified block.
 * Note that within an each/map block, '⛏' and '⚿' refer to the collection.
 *
 * In other languages:
 * Dutch: [Lijst] elk: [Codeblok] | Past het blok code toe op elk paar uit de lijst.
 */
ctr_object* ctr_map_each(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* block = argumentList->object;
	ctr_mapitem* m;
	if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
		CtrStdFlow = ctr_build_string_from_cstring( CTR_ERR_EXP_BLK );
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
		ctr_block_run(block, arguments, myself);
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
 * [Map] has: [Object]
 *
 * Checks whether the map contains the specified value.
 * Note that the object gets converted to a string before
 * comparison. In case of a map or array this means the comparison
 * will be based on the serialized structure.
 * The example will output: True False False False False.
 * 
 * Usage:
 *
 * ☞ shop := (Map new
 *	put: 'magazine' at: 'books',
 *	put: 'computer' at: 'electronics',
 *	put: 'lipstick' at: 'cosmetics'
 * ).
 * ✎ write: (shop has: 'computer'), end.
 * ✎ write: (shop has: 'sausage'), end.
 * ✎ write: (shop has: 'computers'), end.
 * ✎ write: (shop has: 'compute'), end.
 * ✎ write: (shop has: '2computer'), end.
 *
 * In other languages:
 * Dutch: [Lijst] heeft: [Object]
 * Beantwoord de vraag of het object op de lijst staat met Waar of Onwaar.
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
			needle->value.svalue->value, needle->value.svalue->vlen)) {
				found = 1;
			}
		}
		m = m->next;
	}
	return ctr_build_bool(found);
}

/**
 * [Map] string
 *
 * Returns a string representation of a map encoded in Citrine itself.
 * This will give you an
 * evallable representation of the map and all of its members.
 * The sting method is automatically invoked when attempting to
 * print a Map:
 *
 * Usage
 *
 * m := (Map new) put: 'hello' at: 'world'.
 * x := m string.
 * ✎ write: (Map new).
 *
 * In other languages:
 * Dutch: [Lijst] tekst | Geeft tekstuele weergave van lijst (kan weer worden geevalueerd)
 */
ctr_object* ctr_map_to_string( ctr_object* myself, ctr_argument* argumentList) {
	ctr_object*  string;
	ctr_mapitem* mapItem;
	ctr_argument* newArgumentList;
	string  = ctr_build_string_from_cstring( CTR_DICT_CODEGEN_MAP_NEW );
	mapItem = myself->properties->head;
	newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	while( mapItem ) {
		newArgumentList->object = ctr_build_string_from_cstring( CTR_DICT_CODEGEN_MAP_PUT );
		ctr_string_append( string, newArgumentList );
		if ( mapItem->value->info.type == CTR_OBJECT_TYPE_OTBOOL || mapItem->value->info.type == CTR_OBJECT_TYPE_OTNUMBER 
		|| mapItem->value->info.type == CTR_OBJECT_TYPE_OTNIL
		) {
			newArgumentList->object = mapItem->value;
			ctr_string_append( string, newArgumentList );
		} else if ( mapItem->value->info.type == CTR_OBJECT_TYPE_OTSTRING ) {
			newArgumentList->object = ctr_build_string_from_cstring( "'" );
			ctr_string_append( string, newArgumentList );
			newArgumentList->object = ctr_string_quotes_escape( mapItem->value, newArgumentList );
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
		newArgumentList->object = ctr_build_string_from_cstring( CTR_DICT_CODEGEN_MAP_PUT_AT );
		ctr_string_append( string, newArgumentList );
		if ( mapItem->key->info.type == CTR_OBJECT_TYPE_OTBOOL || mapItem->key->info.type == CTR_OBJECT_TYPE_OTNUMBER
		|| mapItem->value->info.type == CTR_OBJECT_TYPE_OTNIL ) {
			newArgumentList->object = mapItem->key;
			ctr_string_append( string, newArgumentList );
		} else if ( mapItem->key->info.type == CTR_OBJECT_TYPE_OTSTRING ) {
			newArgumentList->object = ctr_build_string_from_cstring( "'" );
			ctr_string_append( string, newArgumentList );
			newArgumentList->object = ctr_string_quotes_escape( mapItem->key, newArgumentList );
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
