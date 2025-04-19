#include "citrine.h"

ctr_size ctr_program_length;
uint64_t    ctr_cwlk_subprogram;
int ctr_in_message;




/**
 * @def
 * Nil
 *
 *
 * @test375
 */

char* ctr_international_number(char* old_number, char* new_number) {
	char i, j, k, old_length;
	j = 0;
	k = 0;
	old_length = strlen( old_number );
	for( i = 0; i < old_length; i ++ ) {
		if ( *(old_number + i) == '.' ) {
			strncpy( new_number + j , CTR_DICT_NUM_DEC_SEP, ctr_clex_keyword_num_sep_dec_len);
			j += ctr_clex_keyword_num_sep_dec_len;
			continue;
		}
		if (isdigit(*(old_number + i))) {
			k++;
		}
		*(new_number + j) = *(old_number + i);
		j++;
	}
	return new_number;
}

/**
 * Format a national number to international notation.
 */
char* ctr_national_number(char* old_number, char* new_number) {
	char i, j, old_length;
	j = 0;
	old_length = strlen( old_number );
	for( i = 0; i < old_length; i ++ ) {
		if ( strncmp(old_number+i, CTR_DICT_NUM_THO_SEP, ctr_clex_keyword_num_sep_tho_len) == 0 ) {
			i += (ctr_clex_keyword_num_sep_tho_len - 1);
			continue;
		}
		if ( strncmp(old_number+i, CTR_DICT_NUM_DEC_SEP, ctr_clex_keyword_num_sep_dec_len) == 0 ) {
			i += (ctr_clex_keyword_num_sep_tho_len - 1);
			*(new_number + j) = '.';
			j++;
			continue;
		}
		*(new_number + j) = *(old_number + i);
		j++;
	}
	return new_number;
}



/**
 * @def
 * Nil
 *
 *
 * @test376
 */

ctr_object* ctr_build_nil() {
	return CtrStdNil;
}


ctr_object* ctr_nil_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* nilObject = ctr_build_nil();
	nilObject->link = myself;
	return nilObject;
}


/**
 * @def
 * [ Nil ] Nil?
 *
 *
 * @test377
 */

ctr_object* ctr_nil_is_nil(ctr_object* myself, ctr_argument* argumentList) {
	return CtrStdBoolTrue;
}

/**
 * @def
 * [ Nil ] string
 *
 *
 * @test378
 */

ctr_object* ctr_nil_to_string(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_string_from_cstring( CTR_DICT_NIL );
}

/**
 * @def
 * [ Nil ] else: [ Object ]
 *
 *
 * @test379
 */

ctr_object* ctr_nil_else_set(ctr_object* myself, ctr_argument* argumentList) {
	return argumentList->object;
}

/**
 * @def
 * [ Nil ] number
 *
 *
 * @test380
 */

ctr_object* ctr_nil_to_number(ctr_object* myself, ctr_argument* ctr_argumentList) {
	return ctr_build_number_from_float(0);
}

/**
 * @def
 * [ Nil ] boolean
 *
 *
 * @test381
 */

ctr_object* ctr_nil_to_boolean(ctr_object* myself, ctr_argument* ctr_argumentList) {
	return CtrStdBoolFalse;
}

/**
 * @def
 * Object
 *
 *
 * @test382
 */

ctr_object* ctr_object_make(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* objectInstance = NULL;
	objectInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	objectInstance->link = myself;
	return objectInstance;
}

/**
 * @def
 * [ Object ] type
 *
 *
 * @test383
 */

ctr_object* ctr_object_type(ctr_object* myself, ctr_argument* argumentList) {
	switch(myself->info.type){
		case CTR_OBJECT_TYPE_OTNIL:
			return ctr_build_string_from_cstring( CTR_DICT_NIL );
		case CTR_OBJECT_TYPE_OTBOOL:
			return ctr_build_string_from_cstring( CTR_DICT_BOOLEAN );
		case CTR_OBJECT_TYPE_OTNUMBER:
			return ctr_build_string_from_cstring( CTR_DICT_NUMBER );
		case CTR_OBJECT_TYPE_OTSTRING:
			return ctr_build_string_from_cstring( CTR_DICT_STRING );
		case CTR_OBJECT_TYPE_OTBLOCK:
		case CTR_OBJECT_TYPE_OTNATFUNC:
			return ctr_build_string_from_cstring( CTR_DICT_TASK );
		default:
			return ctr_build_string_from_cstring( CTR_DICT_OBJECT );
	}
}

/**
 * @def
 * [ Object ] code.
 * 
 *
 * @test384
 */

ctr_object* ctr_object_to_code(ctr_object* myself, ctr_argument* argumentList) {
	ctr_argument* newArgumentList;
	ctr_object* string = ctr_build_empty_string();
	newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	newArgumentList->object = ctr_build_string_from_cstring( CTR_DICT_PAREN_OPEN );
	string = ctr_string_append( string, newArgumentList );
	newArgumentList->object = myself;
	string = ctr_string_append( string, newArgumentList );
	newArgumentList->object = ctr_build_string_from_cstring( CTR_DICT_PAREN_CLOSE );
	string = ctr_string_append( string, newArgumentList );
	ctr_heap_free(newArgumentList);
	return string;
}

/**
 * @def
 * [ Object ] string
 *
 *
 * @test385
 */

ctr_object* ctr_object_to_string( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_build_string_from_cstring( CTR_DICT_OBJECT );
}

/**
 * @def
 * [ Object ] number
 *
 *
 * @test386
 */

ctr_object* ctr_object_to_number(ctr_object* myself, ctr_argument* ctr_argumentList) {
	return ctr_build_number_from_float(1);
}

/**
 * @def
 * [ Object ] boolean
 *
 *
 * @test387
 */

ctr_object* ctr_object_to_boolean(ctr_object* myself, ctr_argument* ctr_argumentList) {
	return CtrStdBoolTrue;
}

/**
 * @def
 * [ Object ] equals: [ Object ]
 *
 *
 * @test388
 */

ctr_object* ctr_object_equals(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherObject = argumentList->object;
	if (otherObject == myself) return CtrStdBoolTrue;
	return CtrStdBoolFalse;
}

/**
 * @def
 * [ Object ] ≠ [ Object ]
 *
 *
 * @test389
 */

ctr_object* ctr_object_unequals(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherObject = argumentList->object;
	if (otherObject != myself) return CtrStdBoolTrue;
	return CtrStdBoolFalse;
}

/**
 * @def
 * [ Object ] myself
 *
 *
 * @test390
 */

ctr_object* ctr_object_myself(ctr_object* myself, ctr_argument* argumentList) {
	return myself;
}

/**
 * @def
 * [ Object ] recursive
 *
 *
 * @test391
 */

ctr_object* ctr_object_recursion(ctr_object* myself, ctr_argument* argumentList) {
	ctr_internal_recursion = myself;
	return myself;
}

/**
 * @def
 * [ Object ] do
 *
 *
 * @test392
 */

ctr_object* ctr_object_do( ctr_object* myself, ctr_argument* argumentList ) {
	myself->info.chainMode = 1;
	return myself;
}

/**
 * @def
 * [ Object ] done
 *
 *
 * @test393
 */

ctr_object* ctr_object_done( ctr_object* myself, ctr_argument* argumentList ) {
	myself->info.chainMode = 0;
	return myself;
}

/**
 * @def
 * [ Object ] copy
 *
 *
 * @test394
 */

ctr_object* ctr_bool_copy( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_build_bool(myself->value.bvalue);
}

ctr_object* ctr_number_copy( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_object* qual = ctr_internal_object_find_property( myself, ctr_build_string_from_cstring( CTR_DICT_QUALIFIER ), CTR_CATEGORY_PRIVATE_PROPERTY );
	ctr_object* copy = ctr_build_number_from_float(myself->value.nvalue);
	if ( qual != NULL ) {
		ctr_internal_object_set_property( copy, ctr_build_string_from_cstring( CTR_DICT_QUALIFIER ),  ctr_internal_cast2string(qual), CTR_CATEGORY_PRIVATE_PROPERTY );
	}
	return copy;
}

ctr_object* ctr_string_copy( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_build_string(myself->value.svalue->value, myself->value.svalue->vlen);
}

/**
 * 
 * @def
 * [ Object ] case: [ Object ] do: [ Code ]
 *
 *
 * @test395
 */

ctr_object* ctr_object_case_do( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_object* block = argumentList->next->object;
	ctr_argument* compareArguments;
	if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
		CtrStdFlow = ctr_error( CTR_ERR_EXP_BLK, 0 );
		return CtrStdNil;
	}
	compareArguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	compareArguments->object = argumentList->object;
	compareArguments->next = NULL;
	if (
		ctr_internal_cast2bool(
			ctr_send_message( myself, CTR_DICT_SYMBOL_EQUALS, strlen(CTR_DICT_SYMBOL_EQUALS), compareArguments )
		)->value.bvalue == 1) {
		block->info.sticky = 1;
		int sticky = myself->info.sticky;
		myself->info.sticky = 1;
		ctr_block_run(block, compareArguments, NULL);
		myself->info.sticky = sticky;
		if (CtrStdFlow == CtrStdContinue) CtrStdFlow = NULL; /* consume continue */
		if (CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL; /* consume break */
		block->info.mark = 0;
		block->info.sticky = 0;
	}
	ctr_heap_free( compareArguments );
	return myself;
}

/**
 * @def
 * [ Object ] message: [ String ] arguments: [ List ]
 *
 *
 * @test396
 */

ctr_object* ctr_object_message( ctr_object* myself, ctr_argument* argumentList ) {
	//@important! we use a List because message+arguments would exceed argument limit!
	ctr_object* message = ctr_internal_cast2string( argumentList->object );
	ctr_object* arr     = argumentList->next->object;
	if ( arr->info.type != CTR_OBJECT_TYPE_OTARRAY ) {
		ctr_error( CTR_ERR_EXP_ARR, 0 );
		return CtrStdNil;
	}
	ctr_size length = (int) ctr_array_count( arr,  NULL )->value.nvalue;
	ctr_size i = 0;
	ctr_argument* args = ctr_heap_allocate( sizeof( ctr_argument ) );
	ctr_argument* cur  = args;
	for ( i = 0; i < length; i ++ ) {
		ctr_argument* index = ctr_heap_allocate( sizeof( ctr_argument ) );
		if ( i > 0 ) {
			cur->next = ctr_heap_allocate( sizeof( ctr_argument ) );
			cur = cur->next;
		}
		index->object = ctr_build_number_from_float( (double) i );
		cur->object = ctr_array_get( arr, index );
		ctr_heap_free( index );
	}
	if ( i > 0 ) {
		ctr_argument* closingArgument = ctr_heap_allocate( sizeof( ctr_argument ) );
		closingArgument->object = CtrStdNil;
		cur->next = closingArgument;
	}
	char* flatMessage = ctr_heap_allocate_cstring( message );
	ctr_object* answer = ctr_send_message( myself, flatMessage, message->value.svalue->vlen, args);
	cur = args;
	if ( length == 0 ) {
		ctr_heap_free(args);
	} else {
		for ( i = 0; i <= length; i ++ ) {
			ctr_argument* a = cur;
			if ( i < length ) cur = cur->next;
			ctr_heap_free( a );
		}
	}
	ctr_heap_free( flatMessage );
	return answer;
}

/**
 * @def
 * [ Object ] on: [ String ] do: [ Code ]
 *
 *
 * @test397
 */

ctr_object* ctr_object_on_do(ctr_object* myself, ctr_argument* argumentList) {
	ctr_argument* nextArgument;
	ctr_object* methodBlock;
	ctr_object* methodName = argumentList->object;
	if (methodName->info.type != CTR_OBJECT_TYPE_OTSTRING) {
		CtrStdFlow = ctr_error( CTR_ERR_EXP_STR, 0 );
		return myself;
	}
	nextArgument = argumentList->next;
	methodBlock = nextArgument->object;
	methodBlock->info.selfbind = 1;
	if (methodBlock->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
		CtrStdFlow = ctr_error( CTR_ERR_EXP_BLK, 0 );
		return myself;
	}
	ctr_internal_object_add_property(myself, methodName, methodBlock, 1);
	return myself;
}

/**
 * @def
 * [ Object ] respond: [ String ]
 *
 *
 * @test398
 */

ctr_object* ctr_object_respond(ctr_object* myself, ctr_argument* argumentList) {
	return myself;
}

/**
 * @def
 * [ Object ] respond: [ String ] and: [ String ]
 *
 *
 * @test399
 */

ctr_object* ctr_object_respond_and(ctr_object* myself, ctr_argument* argumentList) {
	return myself;
}

/**
 * @def
 * [ Object ] respond: [ String ] and: [ String ] and: [ String ]
 *
 *
 * @test400
 */

ctr_object* ctr_object_respond_and_and(ctr_object* myself, ctr_argument* argumentList) {
	return myself;
}

ctr_object* ctr_object_respond_and_and_and(ctr_object* myself, ctr_argument* argumentList) {
	return myself;
}

/**
 * @def
 * [ Object ] Nil?
 *
 *
 * @test401
 */

ctr_object* ctr_object_is_nil(ctr_object* myself, ctr_argument* argumentList) {
	return CtrStdBoolFalse;
}

/**
 * @def
 * [ Object ] learn: [ String ] means: [ String ]
 *
 *
 * @test402
 */

ctr_object* ctr_object_learn_meaning(ctr_object* myself, ctr_argument* ctr_argumentList) {
       char*  current_method_name_str;
       ctr_size     current_method_name_len;
       ctr_size     i                      = 0;
       ctr_mapitem* current_method         = myself->methods->head;
       ctr_object*  target_method_name     = ctr_internal_cast2string( ctr_argumentList->next->object );
       char*        target_method_name_str = target_method_name->value.svalue->value;
       ctr_size     target_method_name_len = target_method_name->value.svalue->vlen;
       ctr_object*  alias                  = ctr_internal_cast2string( ctr_argumentList->object );
       while( i < myself->methods->size ) {
               current_method_name_str = current_method->key->value.svalue->value;
               current_method_name_len = current_method->key->value.svalue->vlen;
               if (  current_method_name_len == target_method_name_len ) {
					if ( strncmp( current_method_name_str, target_method_name_str, current_method_name_len ) == 0 ) {
                       ctr_internal_object_add_property( myself, alias, current_method->value, 1);
                       break;
					}
				}
               current_method = current_method->next;
               i ++;
       }
       return myself;
}

/**
 * @def
 * Boolean
 *
 *
 * @test403
 */

ctr_object* ctr_build_bool(int truth) {
	if (truth) return CtrStdBoolTrue;
	return CtrStdBoolFalse;
}

ctr_object* ctr_bool_new(ctr_object* myself, ctr_argument* argumentList) {
	return CtrStdBoolFalse;
}

/**
 * @def
 * [ Boolean ] = [ Boolean ]
 *
 *
 * @test404
 */

ctr_object* ctr_bool_eq(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(ctr_internal_cast2bool(argumentList->object)->value.bvalue == myself->value.bvalue);
}


/**
 * @def
 * [ Boolean ] ≠ [ Boolean ]
 *
 *
 * @test405
 */

ctr_object* ctr_bool_neq(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(ctr_internal_cast2bool(argumentList->object)->value.bvalue != myself->value.bvalue);
}

/**
 * @def
 * [ Boolean ] string
 *
 *
 * @test406
 */

ctr_object* ctr_bool_to_string(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.bvalue == 1) {
		return ctr_build_string_from_cstring( CTR_DICT_TRUE );
	} else {
		return ctr_build_string_from_cstring( CTR_DICT_FALSE );
	}
}

/**
 * @def
 * [ Boolean ] break
 *
 *
 * @test407
 */

ctr_object* ctr_bool_break(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.bvalue) {
		CtrStdFlow = CtrStdBreak; /* If error = Break it's a break, there is no real error. */
	}
	return myself;
}

/**
 * @def
 * [ Boolean ] continue
 *
 *
 * @test408
 */

ctr_object* ctr_bool_continue(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.bvalue) {
		CtrStdFlow = CtrStdContinue; /* If error = Continue, then it breaks only one iteration (return). */
	}
	return myself;
}

/**
 * @def
 * [ Boolean ] true: [ Code ]
 *
 *
 * @test409
 */

ctr_object* ctr_bool_if_true(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.bvalue) {
		ctr_object* codeBlock = argumentList->object;
		if (codeBlock->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
			CtrStdFlow = ctr_error( CTR_ERR_DIVZERO, 0 );
			return myself;
		}
		ctr_argument* arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		arguments->object = myself;
		int sticky = myself->info.sticky;
		myself->info.sticky = 1;
		ctr_block_run(codeBlock, arguments, NULL);
		myself->info.sticky = sticky;
		ctr_heap_free( arguments );
		return myself;
	}
	if (CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL; /* consume break */
	return myself;
}

/**
 * @def
 * [ Boolean ] false: [ Code ]
 *
 *
 * @test410
 */

ctr_object* ctr_bool_if_false(ctr_object* myself, ctr_argument* argumentList) {
	if (!myself->value.bvalue) {
		ctr_object* codeBlock = argumentList->object;
		if (codeBlock->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
			CtrStdFlow = ctr_error( CTR_ERR_DIVZERO, 0 );
			return myself;
		}
		ctr_argument* arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		arguments->object = myself;
		int sticky = myself->info.sticky;
		myself->info.sticky = 1;
		ctr_block_run(codeBlock, arguments, NULL);
		myself->info.sticky = sticky;
		ctr_heap_free( arguments );
		return myself;
	}
	if (CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL; /* consume break */
	return myself;
}

/**
 * @internal
 */
ctr_object* ctr_object_if_false( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_bool_if_false( ctr_internal_cast2bool( myself ), argumentList );
}

/**
 * @internal
 */
ctr_object* ctr_object_if_true( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_bool_if_true( ctr_internal_cast2bool( myself ), argumentList );
}

/**
 * @def
 * [ Boolean ] not
 *
 *
 * @test411
 */

ctr_object* ctr_bool_not(ctr_object* myself, ctr_argument* argumentList) {
	if (!myself->value.bvalue) return CtrStdBoolTrue;
	return CtrStdBoolFalse;
}

/**
 * @def
 * [ Boolean ] either: [ Object ] or: [ Object ]
 *
 *
 * @test412
 */

ctr_object* ctr_bool_either_or(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.bvalue) {
		return argumentList->object;
	} else {
		return argumentList->next->object;
	}
}

/**
 * @def
 * [ Boolean ] &&: [ Boolean ]
 * 
 *
 * @test413
 */

ctr_object* ctr_bool_and(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* other = ctr_internal_cast2bool(argumentList->object);
	if ((myself->value.bvalue && other->value.bvalue)) return CtrStdBoolTrue;
	return CtrStdBoolFalse;
}

/**
 * @def
 * [ Boolean ] nor: [ Boolean ]
 *
 *
 * @test414
 */

ctr_object* ctr_bool_nor(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* other = ctr_internal_cast2bool(argumentList->object);
	return ctr_build_bool((!myself->value.bvalue && !other->value.bvalue));
}

/**
 * @def
 * [ Boolean ] ||: [ Boolean ]
 *
 *
 * @test415
 */

ctr_object* ctr_bool_or(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* other = ctr_internal_cast2bool(argumentList->object);
	return ctr_build_bool((myself->value.bvalue || other->value.bvalue));
}

/**
 * @def
 * [ Boolean ] number
 *
 *
 * @test416
 */

ctr_object* ctr_bool_to_number(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float( (ctr_number) myself->value.bvalue );
}

/**
 * @def
 * Number
 *
 *
 * @test417
 */

ctr_object* ctr_build_number(char* n) {
	ctr_object* numberObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNUMBER);
	numberObject->value.nvalue = atof(n);
	numberObject->link = CtrStdNumber;
	return numberObject;
}

ctr_object* ctr_number_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* numberObject = ctr_build_number_from_float(0);
	numberObject->link = myself;
	return numberObject;
}

/**
 * @internal
 * BuildNumberFromString
 */
ctr_object* ctr_build_number_from_string(char* str, ctr_size length, char international) {
	char* old_number;
	char* new_number;
	ctr_object* numberObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNUMBER);
	/* turn string into a C-string before feeding it to atof */
	int stringNumberLength = ( length <= 40 ) ? length : 40;
	/* max length is 40 (and that's probably even too long... ) */
	old_number = (char*) ctr_heap_allocate( 41 * sizeof( char ) );
	memcpy( old_number, str, stringNumberLength );
	/* convert national number to international number */
	if (international) {
		new_number = (char*) ctr_heap_allocate( 41 * sizeof( char ) );
		ctr_national_number( old_number, new_number );
		numberObject->value.nvalue = atof(new_number);
		ctr_heap_free( new_number );
	} else {
		numberObject->value.nvalue = atof(old_number);
	}
	numberObject->link = CtrStdNumber;
	ctr_heap_free( old_number );
	return numberObject;
}

/**
 * @internal
 * BuildNumberFromFloat
 *
 * Creates a number object from a float.
 * Internal use only.
 */
ctr_object* ctr_build_number_from_float(ctr_number f) {
	ctr_object* numberObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNUMBER);
	numberObject->value.nvalue = f;
	numberObject->link = CtrStdNumber;
	return numberObject;
}

/**
 * @def
 * [ Number ] > [ Number ]
 *
 *
 * @test418
 */

ctr_object* ctr_number_higherThan(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue > otherNum->value.nvalue));
}

/**
 * @def
 * [ Number ] ≥ [ Number ]
 *
 *
 * @test419
 */

ctr_object* ctr_number_higherEqThan(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue >= otherNum->value.nvalue));
}

/**
 * @def
 * [ Number ] < [ Number ]
 * 
 *
 * @test420
 */

ctr_object* ctr_number_lowerThan(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue < otherNum->value.nvalue));
}

/**
 * @def
 * [ Number ] ≤ [ Number ]
 * 
 *
 * @test421
 */

ctr_object* ctr_number_lowerEqThan(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue <= otherNum->value.nvalue));
}

/**
 * @def
 * [ Number ] = [ Number ]
 *
 *
 * @test422
 */

ctr_object* ctr_number_eq(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool(myself->value.nvalue == otherNum->value.nvalue);
}

/**
 * @def
 * [ Number ] ≠ [ Number ]
 *
 *
 * @test423
 */

ctr_object* ctr_number_neq(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool(myself->value.nvalue != otherNum->value.nvalue);
}

/**
 * @def
 * [ Number ] between: [ Number ] and: [ Number ]
 * 
 *
 * @test424
 */

ctr_object* ctr_number_between(ctr_object* myself, ctr_argument* argumentList) {
	ctr_number upper_bound;
	ctr_number lower_bound;
	ctr_number upper_bound_from_zero;
	ctr_number swap;
	lower_bound = ctr_internal_cast2number(
		argumentList->object
	)->value.nvalue;
	upper_bound = ctr_internal_cast2number(
		argumentList->next->object
	)->value.nvalue;
	lower_bound = round(lower_bound);
	upper_bound = round(upper_bound);
	if (lower_bound > upper_bound) {
		swap = lower_bound;
		lower_bound = upper_bound;
		upper_bound = swap;
	}
	if ( lower_bound == upper_bound ) {
		return ctr_build_number_from_float( lower_bound );
	}
	upper_bound_from_zero = abs( (int)upper_bound - (int)lower_bound);
	int rolls = ceil(upper_bound_from_zero / RAND_MAX);
	int64_t result = 0;
	while(rolls-- > 0) {
		result += rand();
	}
	return ctr_build_number_from_float(
		(ctr_number) (int64_t) (result % ((int64_t) upper_bound_from_zero + 1)) + (int64_t) lower_bound
	);
}

/**
 * @def
 * [ Number ] odd?
 *
 *
 * @test425
 */

ctr_object* ctr_number_odd(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool((int)myself->value.nvalue % 2);
}

/**
 * @def
 * [ Number ] even?
 *
 *
 * @test426
 */

ctr_object* ctr_number_even(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(!((int)myself->value.nvalue % 2));
}

/**
 * @def
 * [ Number ] + [ Number ]
 *
 *
 * @test427
 */

ctr_object* ctr_number_add(ctr_object* myself, ctr_argument* argumentList) {
	ctr_argument* newArg;
	ctr_object* otherNum = argumentList->object;
	ctr_object* result;
	ctr_number a;
	ctr_number b;
	ctr_object* strObject;
	if (otherNum->info.type == CTR_OBJECT_TYPE_OTSTRING) {
		strObject = ctr_internal_cast2string(myself);
		newArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		newArg->object = otherNum;
		result = ctr_string_concat(strObject, newArg);
		ctr_heap_free( newArg );
		return result;
	} else {
		otherNum = ctr_internal_cast2number( otherNum );
	}
	a = myself->value.nvalue;
	b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a+b));
}

/**
 * @def
 * [ Number ] add: [ Number ]
 *
 *
 * @test428
 */

ctr_object* ctr_number_inc(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	myself->value.nvalue += otherNum->value.nvalue;
	return myself;
}

/**
 * @def
 * [ Number ] - [ Number ]
 *
 *
 * @test429
 */

ctr_object* ctr_number_minus(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_number a = myself->value.nvalue;
	ctr_number b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a-b));
}

/**
 * @def
 * [ Number ] subtract: [ Number ]
 *
 *
 * @test430
 */

ctr_object* ctr_number_dec(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	myself->value.nvalue -= otherNum->value.nvalue;
	return myself;
}

/**
 * @def
 * [ Number ] × [ Number ]
 * 
 *
 * @test431
 */

ctr_object* ctr_number_multiply(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum;
	ctr_number a;
	ctr_number b;
	otherNum = ctr_internal_cast2number(argumentList->object);
	a = myself->value.nvalue;
	b = otherNum->value.nvalue;
	return ctr_build_number_from_float(a*b);
}

/**
 * @def
 * [ Code ] × [ Number ]
 *
 *
 * @test432
 */

ctr_object* ctr_block_times(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* indexNumber;
	ctr_object* block = myself;
	ctr_argument* arguments;
	int t;
	int i;
	block->info.sticky = 1;
	t = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	for(i=0; i<t; i++) {
		indexNumber = ctr_build_number_from_float((ctr_number) i);
		arguments->object = indexNumber;
		ctr_block_run(block, arguments, NULL);
		if (CtrStdFlow == CtrStdContinue) CtrStdFlow = NULL; /* consume continue */
		if (CtrStdFlow) break;
	}
	ctr_heap_free( arguments );
	if (CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL; /* consume break */
	block->info.mark = 0;
	block->info.sticky = 0;
	return myself;
}

/**
 * @def
 * [ Code ] procedure
 *
 *
 * @test433
 */

ctr_object* ctr_block_procedure(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* block = myself;
	ctr_argument* arguments;
	block->info.sticky = 1;
	arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	arguments->object = ctr_build_number_from_float((ctr_number) 1);
	ctr_block_run(block, arguments, NULL);
	ctr_heap_free( arguments );
	if (CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL; /* consume break */
	block->info.mark = 0;
	block->info.sticky = 0;
	return myself;
}

/**
 * @def
 * [ Number ] multiply by: [ Number ]
 * 
 *
 * @test434
 */

ctr_object* ctr_number_mul(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	myself->value.nvalue *= otherNum->value.nvalue;
	return myself;
}

/**
 * @def
 * [ Number ] ÷ [ Number ]
 *
 *
 * @test435
 */

ctr_object* ctr_number_divide(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_number a = myself->value.nvalue;
	ctr_number b = otherNum->value.nvalue;
	if (b == 0) {
		CtrStdFlow = ctr_error( CTR_ERR_DIVZERO, 0 );
		return myself;
	}
	return ctr_build_number_from_float((a/b));
}

/**
 * @def
 * [ Number ] divide by: [ Number ]
 *
 *
 * @test436
 */

ctr_object* ctr_number_div(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	if (otherNum->value.nvalue == 0) {
		CtrStdFlow = ctr_error( CTR_ERR_DIVZERO, 0 );
		return myself;
	}
	myself->value.nvalue /= otherNum->value.nvalue;
	return myself;
}

/**
 * @def
 * [ Number ] modulo: [ modulo ]
 *
 *
 * @test437
 */

ctr_object* ctr_number_modulo(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_number a = myself->value.nvalue;
	ctr_number b = otherNum->value.nvalue;
	if (b == 0) {
		CtrStdFlow = ctr_error( CTR_ERR_DIVZERO, 0 );
		return myself;
	}
	return ctr_build_number_from_float(fmod(a,b));
}

/**
 * @def
 * [ Number ] power: [ Number ]
 * 
 *
 * @test438
 */

ctr_object* ctr_number_pow(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_number a = myself->value.nvalue;
	ctr_number b = otherNum->value.nvalue;
	return ctr_build_number_from_float(pow(a,b));
}

/**
 * @def
 * [ Number ] positive?
 *
 *
 * @test439
 */

ctr_object* ctr_number_positive(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool( ( myself->value.nvalue > 0) );
}

/**
 * @def
 * [ Number ] negative?
 * 
 *
 * @test440
 */

ctr_object* ctr_number_negative(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool( ( myself->value.nvalue < 0) );
}

/**
 * @def
 * [ Number ] floor
 *
 *
 * @test441
 */

ctr_object* ctr_number_floor(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(floor(myself->value.nvalue));
}

/**
 * @def
 * [ Number ] [ String ]
 *
 *
 * @test442
 */

ctr_object* ctr_number_qualify(ctr_object* myself, ctr_argument* argumentList) {
	ctr_internal_object_set_property( myself, ctr_build_string_from_cstring( CTR_DICT_QUALIFIER ), ctr_internal_cast2string( argumentList->object ), CTR_CATEGORY_PRIVATE_PROPERTY );
	return myself;
}

/**
 * @def
 * [ Number ] qualifier.
 *
 *
 * @test443
 */

ctr_object* ctr_number_qualification(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* answer = ctr_internal_object_find_property( myself, ctr_build_string_from_cstring( CTR_DICT_QUALIFIER ), CTR_CATEGORY_PRIVATE_PROPERTY );
	if ( answer == NULL ) return CtrStdNil;
	return answer;
}

/**
 * @internal
 */
ctr_object* ctr_number_respond_to(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_number_qualify( myself, argumentList );
}

/**
 * @def
 * [ Number ] ceil
 * 
 *
 * @test444
 */

ctr_object* ctr_number_ceil(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(ceil(myself->value.nvalue));
}

/**
 * @def
 * [ Number ] round
 * 
 *
 * @test445
 */

ctr_object* ctr_number_round(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(round(myself->value.nvalue));
}

/**
 * @def
 * [ Number ] absolute
 *
 *
 * @test446
 */

ctr_object* ctr_number_abs(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(fabs(myself->value.nvalue));
}

/**
 * @def
 * [ Number ] square root
 *
 *
 * @test447
 */

ctr_object* ctr_number_sqrt(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(sqrt(myself->value.nvalue));
}

/**
 * @def
 * [ Number ] sin
 *
 *
 * @test448
 */

ctr_object* ctr_number_sin(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(sin(myself->value.nvalue));
}

/**
 * @def
 * [ Number ] cos
 *
 *
 * @test449
 */

ctr_object* ctr_number_cos(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(cos(myself->value.nvalue));
}

/**
 * @def
 * [ Number ] tan
 *
 *
 * @test450
 */

ctr_object* ctr_number_tan(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(tan(myself->value.nvalue));
}

/**
 * @def
 * [ Number ] atan
 *
 *
 * @test451
 */

ctr_object* ctr_number_atan(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(atan(myself->value.nvalue));
}

/**
 * @def
 * [ Number ] log
 *
 *
 * @test452
 */

ctr_object* ctr_number_log(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(log(myself->value.nvalue));
}

/**
 * @def
 * [ Number ] & [ Number ]
 *
 *
 * @test453
 */

ctr_object* ctr_number_bit_and(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float((int)myself->value.nvalue & (int)ctr_tonum(argumentList->object));
}

/**
 * @def
 * [ Number ] | [ Number ]
 *
 *
 * @test454
 */

ctr_object* ctr_number_bit_or(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float((int)myself->value.nvalue | (int)ctr_tonum(argumentList->object));
}

/**
 * @def
 * [ Number ] ^ [ Number ]
 *
 *
 * @test455
 */

ctr_object* ctr_number_bit_xor(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float((int)myself->value.nvalue ^ (int)ctr_tonum(argumentList->object));
}

/**
 * @internal
 * Generic method, used by:
 * - ctr_number_to_string
 * - ctr_number_to_string_flat
 */
ctr_object* ctr_internal_number_to_string(ctr_object* myself, ctr_argument* argumentList, char flat) {
	ctr_object* o = myself;
	char* q;
	char* p;
	char* buf;
	int bufSize;
	ctr_object* qname;
	ctr_object* qual;
	ctr_object* stringObject;
	if (!flat) {
		qname = ctr_build_string_from_cstring( CTR_DICT_QUALIFIER );
		qual = ctr_internal_object_find_property( myself, qname, CTR_CATEGORY_PRIVATE_PROPERTY );
		if (qual) {
			qual = ctr_internal_cast2string( qual );
			q = ctr_heap_allocate( (100 + qual->value.svalue->vlen) * sizeof( char ) );
		} else {
			q = ctr_heap_allocate( 100 * sizeof( char ) );
		}
	}
	bufSize = 100 * sizeof( char );
	buf = ctr_heap_allocate( bufSize );
	snprintf( buf, 99, "%.10f", o->value.nvalue );
	p = buf + strlen(buf) - 1;
	while ( *p == '0' && *p-- != '.' );
	*( p + 1 ) = '\0';
	if ( *p == '.' ) *p = '\0';
	if (!flat) {
		q = ctr_international_number( buf, q );
		if (qual) {
			strcat(q, " ");
			strncat(q, qual->value.svalue->value, qual->value.svalue->vlen);
		}
		stringObject = ctr_build_string_from_cstring(q);
		ctr_heap_free( q );
	} else {
		stringObject = ctr_build_string_from_cstring(buf);
	}
	ctr_heap_free( buf );
	return stringObject;
}

/**
 * @def
 * [ Number ] string
 *
 *
 * @test456
 */

ctr_object* ctr_number_to_string(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_number_to_string(myself, argumentList, 0);
}

/**
 * @def
 * [ Number ] raw
 *
 *
 * @test457
 */

ctr_object* ctr_number_to_string_flat(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_number_to_string(myself, argumentList, 1);
}

/**
 * @def
 * [ Number ] boolean
 *
 *
 * @test458
 */

ctr_object* ctr_number_to_boolean(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool( myself->value.nvalue );
}

/**
 * @def
 * String
 *
 *
 * @test459
 */

ctr_object* ctr_build_string(char* stringValue, ctr_size size) {
	ctr_object* stringObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTSTRING);
	if (size != 0) {
		stringObject->value.svalue->value = ctr_heap_allocate( size*sizeof(char) );
		memcpy(stringObject->value.svalue->value, stringValue, ( sizeof(char) * size ) );
	}
	stringObject->value.svalue->vlen = size;
	stringObject->link = CtrStdString;
	return stringObject;
}

/**
 * @internal
 * BuildStringFromCString
 *
 * Creates a Citrine String from a 0 terminated C String.
 */
ctr_object* ctr_build_string_from_cstring(char* cstring) {
	return ctr_build_string(cstring, strlen(cstring));
}

/**
 * @internal
 * BuildEmptyString
 *
 * Creates an empty string object, use this to avoid using
 * the 'magic' number 0 when building a string, it is more
 * readable this way and your intention is clearer.
 */
ctr_object* ctr_build_empty_string() {
	return ctr_build_string( "", 0 );
}

/**
 * @def
 * [ String ] object
 *
 *
 * @test460
 */

ctr_object* ctr_string_eval(ctr_object* myself, ctr_argument* argumentList) {
	ctr_tnode* parsedCode;
	ctr_object* result;
	char* prg;
	int ctr_callstack_index_old = ctr_callstack_index;
	prg = ctr_heap_allocate_cstring(myself);
	ctr_program_length = strlen(prg);
	size_t memblock = ctr_heap_tracker_memoryblocknumber();
	ctr_clex_load(prg);
	ctr_source_mapping = 0;
	parsedCode = ctr_cparse_expr(0);
	if (parsedCode == NULL) {
		ctr_heap_free( prg );
		return CtrStdNil;
	}
	ctr_heap_free( prg );
	ctr_cwlk_subprogram++;
	char r;
	ctr_deserialize_mode = 1;
	result = ctr_cwlk_expr(parsedCode,&r);
	ctr_deserialize_mode = 0;
	ctr_source_mapping = 1;
	ctr_callstack_index = ctr_callstack_index_old;
	ctr_cwlk_subprogram--;
	ctr_heap_tracker_rewind(memblock);
	if (result == NULL) {
		return CtrStdNil;
	}
	return result;
}



/**
 * @def
 * [ String ] = [ String ]
 * 
 *
 * @test461
 */

ctr_object* ctr_string_eq(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* other = ctr_internal_cast2string( argumentList->object );
	if (other->value.svalue->vlen != myself->value.svalue->vlen) {
		return CtrStdBoolFalse;
	}
	return ctr_build_bool((strncmp(other->value.svalue->value, myself->value.svalue->value, myself->value.svalue->vlen)==0));
}

/**
 * @def
 * [ String ] ≠ [ String ]
 * 
 *
 * @test462
 */

ctr_object* ctr_string_neq(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* other = ctr_internal_cast2string( argumentList->object );
	if (other->value.svalue->vlen != myself->value.svalue->vlen) {
		return CtrStdBoolTrue;
	}
	return ctr_build_bool(!(strncmp(other->value.svalue->value, myself->value.svalue->value, myself->value.svalue->vlen)==0));
}

/**
 * @def
 * [ String ] length
 *
 *
 * @test463
 */

ctr_object* ctr_string_length(ctr_object* myself, ctr_argument* argumentList) {
	ctr_size n = ctr_getutf8len(myself->value.svalue->value, (ctr_size) myself->value.svalue->vlen);
	return ctr_build_number_from_float((ctr_number) n);
}

/**
 * @def
 * [ String ] bytes
 *
 *
 * @test464
 */

ctr_object* ctr_string_bytes(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(myself->value.svalue->vlen);
}

/**
 * @def
 * [ String ] + [ String ]
 *
 *
 * @test465
 */

ctr_object* ctr_string_concat(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* strObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTSTRING);
	ctr_size n1;
	ctr_size n2;
	char* dest;
	ctr_object* newString;
	strObject = ctr_internal_cast2string(argumentList->object);
	n1 = myself->value.svalue->vlen;
	n2 = strObject->value.svalue->vlen;
	dest = ctr_heap_allocate( sizeof(char) * ( n1 + n2 ) );
	memcpy(dest, myself->value.svalue->value, n1);
	memcpy(dest+n1, strObject->value.svalue->value, n2);
	newString = ctr_build_string(dest, (n1 + n2));
	ctr_heap_free( dest );
	return newString;
}

/**
 * @def
 * [ String ] append: [ String ]
 *
 *
 * @test466
 */

ctr_object* ctr_string_append(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* strObject;
	ctr_size n1;
	ctr_size n2;
	char* dest;
	ctr_gc_internal_pin(myself);
	strObject = ctr_internal_cast2string(argumentList->object);
	n1 = myself->value.svalue->vlen;
	n2 = strObject->value.svalue->vlen;
	if ( ( n1 + n2 ) == 0 ) return myself;
	dest = ctr_heap_allocate( sizeof( char ) * ( n1 + n2 ) );
	memcpy(dest, myself->value.svalue->value, n1);
	memcpy(dest+n1, strObject->value.svalue->value, n2);
	if ( n1 > 0 ) {
		ctr_heap_free( myself->value.svalue->value );
	}
	myself->value.svalue->value = dest;
	myself->value.svalue->vlen  = (n1 + n2);
	return myself;
}


ctr_object* ctr_string_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* stringObject = ctr_build_empty_string();
	stringObject->link = myself;
	return stringObject;
}

/**
 * @def
 * [ String ] code
 *
 *
 * @test467
 */

ctr_object* ctr_string_to_code(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* answer;
	ctr_size i;
	ctr_size j;
	ctr_size q;
	char* str;
	char* quotes = (char*) ctr_heap_allocate( myself->value.svalue->vlen );
	ctr_size len = myself->value.svalue->vlen;
	j = 0;
	for(i = 0; i < myself->value.svalue->vlen; i++ ) {
		if (
			strncmp((myself->value.svalue->value + i),  CTR_DICT_QUOT_OPEN,  ctr_clex_keyword_qo_len)==0 ||
			strncmp((myself->value.svalue->value + i),  CTR_DICT_QUOT_CLOSE, ctr_clex_keyword_qc_len)==0
		) {
			len++;
			quotes[j++] = i;
		}
	}
	str = (char*) ctr_heap_allocate( len + ctr_clex_keyword_qo_len + ctr_clex_keyword_qc_len + 1 );
	q = j; j = 0;
	strcpy(str, CTR_DICT_QUOT_OPEN);
	for(i = 0; i < myself->value.svalue->vlen; i++ ) {
		if (j < q && i == quotes[j]) {
			str[ctr_clex_keyword_qo_len + i + j] = '\\';
			j++;
		}
		str[ctr_clex_keyword_qo_len + i + j] = myself->value.svalue->value[i];
	}
	strcpy((str + ctr_clex_keyword_qo_len + i + j), CTR_DICT_QUOT_CLOSE);
	answer = ctr_build_string_from_cstring( str );
	ctr_heap_free(quotes);
	ctr_heap_free(str);
	return answer;
}

/**
 * @def
 * [ String ] from: [ Number ] length: [ Number ]
 *
 *
 * @test468
 */

ctr_object* ctr_string_from_length(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* fromPos = ctr_internal_cast2number(argumentList->object);
	ctr_object* length = ctr_internal_cast2number(argumentList->next->object);
	long len = myself->value.svalue->vlen;
	long a = (fromPos->value.nvalue);
	long b = (length->value.nvalue);
	long ua, ub;
	char* dest;
	if (b == 0 || len == 0) {
		if (len) {
			ctr_heap_free( myself->value.svalue->value );
		}
		myself->value.svalue->value = "";
		myself->value.svalue->vlen = 0;
		return myself;
	}
	if (b < 0) {
		a = a + b;
		b = labs(b);
	}
	if (a < 0) a = 0;
	if (a > len) a = len;
	if ((a + b)>len) b = len - a;
	if ((a + b)<0) b = b - a;
	ua = getBytesUtf8(myself->value.svalue->value, 0, a);
	ub = getBytesUtf8(myself->value.svalue->value, ua, b);
	if (ub == 0) {
		if (len) {
			ctr_heap_free( myself->value.svalue->value );
		}
		myself->value.svalue->value = "";
		myself->value.svalue->vlen = 0;
		return myself;
	}
	dest = ctr_heap_allocate( ub * sizeof(char) );
	memcpy(dest, (myself->value.svalue->value) + ua, ub);
	ctr_heap_free( myself->value.svalue->value );
	myself->value.svalue->value = dest;
	myself->value.svalue->vlen  = ub;
    return myself;
}

/**
 * @def
 * [ String ] offset: [ Number ]
 *
 *
 * @test469
 */

ctr_object* ctr_string_skip(ctr_object* myself, ctr_argument* argumentList) {
	ctr_argument* argument1;
	ctr_argument* argument2;
	ctr_object* result;
	ctr_size textLength;
	ctr_number a = argumentList->object->value.nvalue;
	if (myself->value.svalue->vlen < a) return ctr_build_empty_string();
	argument1 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	argument2 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	argument1->object = ctr_build_number_from_float( a );
	argument1->next = argument2;
	textLength = ctr_getutf8len(myself->value.svalue->value, (ctr_size) myself->value.svalue->vlen);
	argument2->object = ctr_build_number_from_float(textLength - a);
	result = ctr_string_from_length(myself, argument1);
	ctr_heap_free( argument1 );
	ctr_heap_free( argument2 );
	return result;
}

/**
 * @def
 * [ String ] character: [ Number ]
 *
 *
 * @test470
 */

ctr_object* ctr_string_at(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* fromPos = ctr_internal_cast2number(argumentList->object);
	ctr_size a = (ctr_size) (fromPos->value.nvalue);
	ctr_size textLength = ctr_getutf8len(myself->value.svalue->value, (ctr_size) myself->value.svalue->vlen);
	if (a < 0) return CtrStdNil;
	if (a >= textLength) return CtrStdNil;
	long ua = getBytesUtf8(myself->value.svalue->value, 0, a);
	long ub = getBytesUtf8(myself->value.svalue->value, ua, 1);
	ctr_object* newString;
	char* dest = ctr_heap_allocate( ub * sizeof( char ) );
	memcpy(dest, (myself->value.svalue->value) + ua, ub);
	newString = ctr_build_string(dest,ub);
	ctr_heap_free( dest );
	return newString;
}

/**
 * @def
 * [ String ] find: [ String ]
 *
 *
 * @test471
 */

ctr_object* ctr_string_index_of(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* sub = ctr_internal_cast2string(argumentList->object);
	long hlen = myself->value.svalue->vlen;
	long nlen = sub->value.svalue->vlen;
	uintptr_t byte_index;
	ctr_size uchar_index;
	char* p = ctr_internal_memmem(myself->value.svalue->value, hlen, sub->value.svalue->value, nlen, 0);
	if (p == NULL) return ctr_build_nil();
	byte_index = (uintptr_t) p - (uintptr_t) (myself->value.svalue->value);
	uchar_index = ctr_getutf8len(myself->value.svalue->value, byte_index);
	return ctr_build_number_from_float((ctr_number) uchar_index);
}

/**
 * @def
 * [ String ] uppercase
 *
 *
 * @test472
 */

ctr_object* ctr_string_to_upper(ctr_object* myself, ctr_argument* argumentList) {
	char* str = myself->value.svalue->value;
	size_t  len = myself->value.svalue->vlen;
	char* tstr = ctr_heap_allocate( len * sizeof( char ) );
	ctr_size i=0;
	for(i =0; i < len; i++) {
		tstr[i] = toupper(str[i]);
	}
	ctr_heap_free( myself->value.svalue->value );
	myself->value.svalue->value = tstr;
	myself->value.svalue->vlen  = len;
	return myself;
}


/**
 * @def
 * [ String ] lowercase
 *
 *
 * @test473
 */

ctr_object* ctr_string_to_lower(ctr_object* myself, ctr_argument* argumentList) {
	char* str = myself->value.svalue->value;
	size_t len = myself->value.svalue->vlen;
	char* tstr = ctr_heap_allocate( len * sizeof( char ) );
	ctr_size i=0;
	for(i =0; i < len; i++) {
		tstr[i] = tolower(str[i]);
	}
	ctr_heap_free( myself->value.svalue->value );
	myself->value.svalue->value = tstr;
	myself->value.svalue->vlen  = len;
	return myself;
}

/**
 * @def
 * [ String ] last: [ String ]
 *
 *
 * @test474
 */

ctr_object* ctr_string_last_index_of(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* sub = ctr_internal_cast2string(argumentList->object);
	ctr_size hlen = myself->value.svalue->vlen;
	ctr_size nlen = sub->value.svalue->vlen;
	ctr_size uchar_index;
	ctr_size byte_index;
	char* p = ctr_internal_memmem( myself->value.svalue->value, hlen, sub->value.svalue->value, nlen, 1 );
	if (p == NULL) return ctr_build_nil();
	byte_index = (ctr_size) ( p - (myself->value.svalue->value) );
	uchar_index = ctr_getutf8len(myself->value.svalue->value, byte_index);
	return ctr_build_number_from_float((float) uchar_index);
}

/**
 * @def
 * [ String ] [ String ]: [ String ]
 *
 *
 * @test475
 */

ctr_object* ctr_string_fill_in(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* message = ctr_internal_cast2string( argumentList->object );
	ctr_object* slot;

	if ( message->value.svalue->value[message->value.svalue->vlen - 1] == ctr_clex_param_prefix_char ) {
		slot = ctr_build_string( message->value.svalue->value, message->value.svalue->vlen - 1);
	} else {
		slot = message;
	}
	argumentList->object = slot;
	return ctr_string_replace_with( myself, argumentList );
}

/**
 * @def
 * [ String ] replace: [ String ] with: [ String ]
 *
 *
 * @test476
 */

ctr_object* ctr_string_replace_with(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* needle = ctr_internal_cast2string(argumentList->object);
	ctr_object* replacement = ctr_internal_cast2string(argumentList->next->object);
	char* dest;
	char* odest;
	char* src = myself->value.svalue->value;
	char* ndl = needle->value.svalue->value;
	char* rpl = replacement->value.svalue->value;
	long hlen = myself->value.svalue->vlen;
	long nlen = needle->value.svalue->vlen;
	long rlen = replacement->value.svalue->vlen;
	long dlen = hlen;
	char* p;
	long i = 0;
	long offset = 0;
	long d;
	if (nlen == 0 || hlen == 0) {
		return myself;
	}
	dest = (char*) ctr_heap_allocate( dlen * sizeof( char ) );
	odest = dest;
	while(1) {
		p = ctr_internal_memmem(src, hlen, ndl, nlen, 0);
		if (p == NULL) break;
		d = (dest - odest);
		if ((dlen - nlen + rlen)>dlen) {
			dlen = (dlen - nlen + rlen);
			odest = (char*) ctr_heap_reallocate(odest, dlen * sizeof(char) );
			dest = (odest + d);
		} else {
			dlen = (dlen - nlen + rlen);
		}
		offset = (p - src);
		memcpy(dest, src, offset);
		dest = dest + offset;
		memcpy(dest, rpl, rlen);
		dest = dest + rlen;
		hlen = hlen - (offset + nlen);
		src  = src + (offset + nlen);
		i++;
	}
	memcpy(dest, src, hlen);
	ctr_heap_free( myself->value.svalue->value );
	myself->value.svalue->value = odest;
	myself->value.svalue->vlen  = dlen;
	return myself;
}

/**
 * @def
 * [ String ] - [ String ]
 *
 *
 * @test477
 */

ctr_object* ctr_string_minus(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* needle = ctr_internal_cast2string(argumentList->object);
	long nlen = needle->value.svalue->vlen;
	long hlen = myself->value.svalue->vlen;
	if (nlen > hlen) {
		return myself;
	}
	if (memcmp(myself->value.svalue->value + (hlen - nlen), needle->value.svalue->value,nlen)==0) {
		return ctr_build_string(myself->value.svalue->value, hlen - nlen);
	}
	return myself;
}


/**
 * @def
 * [ String ] contains: [ String ]
 *
 *
 * @test478
 */

ctr_object* ctr_string_contains( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_build_bool(
		ctr_string_index_of( myself, argumentList ) != CtrStdNil
	);
}

/**
 * @def
 * [ String ] trim
 *
 *
 * @test479
 */

ctr_object* ctr_string_trim(ctr_object* myself, ctr_argument* argumentList) {
	char* str = myself->value.svalue->value;
	long  len = myself->value.svalue->vlen;
	long i, begin, end, tlen;
	char* tstr;
	if (len == 0) return ctr_build_empty_string();
	i = 0;
	while(i < len && isspace(*(str+i))) i++;
	if (i == len) return ctr_build_empty_string();
	begin = i;
	i = len - 1;
	while(i > begin && isspace(*(str+i))) i--;
	end = i + 1;
	tlen = (end - begin);
	tstr = ctr_heap_allocate( tlen * sizeof( char ) );
	memcpy(tstr, str+begin, tlen);
	ctr_heap_free( myself->value.svalue->value );
	myself->value.svalue->value = tstr;
	myself->value.svalue->vlen  = tlen;
	return myself;
}

/**
 * @def
 * [ String ] number
 *
 *
 * @test480
 */

ctr_object* ctr_string_to_number(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_string(myself->value.svalue->value, myself->value.svalue->vlen, 1);
}

ctr_object* ctr_string_in_to_number(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_string(myself->value.svalue->value, myself->value.svalue->vlen, 0);
}

/**
 * @def
 * [ String ] boolean
 *
 *
 * @test481
 */

ctr_object* ctr_string_to_boolean(ctr_object* myself, ctr_argument* argumentList) {
	if ( myself->value.svalue->vlen == 0 ) return CtrStdBoolFalse;
	return ctr_build_bool( 1 );
}

/**
 * @def
 * [ String ] split: [ String ]
 *
 *
 * @test482
 */

ctr_object* ctr_string_split(ctr_object* myself, ctr_argument* argumentList) {
	char* str = myself->value.svalue->value;
	long len = myself->value.svalue->vlen;
	ctr_object* delimObject  = ctr_internal_cast2string(argumentList->object);
	char* dstr = delimObject->value.svalue->value;
	long dlen = delimObject->value.svalue->vlen;
	ctr_argument* arg;
	ctr_object* arr = ctr_array_new(CtrStdArray, NULL);
	long i;
	long j = 0;
	long k = 0;
	char* buffer = ctr_heap_allocate( sizeof(char)*len );
	for(i=0; i<len; i++) {
		buffer[j] = str[i];
		if (dlen>0 && buffer[j]==dstr[k]) {
			k++;
			if (k>=dlen) {
				arg = ctr_heap_allocate( sizeof( ctr_argument ) );
				arg->object = ctr_build_string(buffer, j+1-dlen);
				ctr_array_push(arr, arg);
				ctr_heap_free( arg );
				j=0;
				k=0;
			} else {
				j++;
			}
		} else {
			k = 0;
			j++;
		}
	}
	arg = ctr_heap_allocate( sizeof( ctr_argument ) );
	arg->object = ctr_build_string(buffer, j);
	ctr_array_push(arr, arg);
	ctr_heap_free( arg );
	ctr_heap_free( buffer );
	return arr;
}

/**
 * @def
 * [ String ] characters
 *
 *
 * @test483
 */

ctr_object* ctr_string_characters( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_size i;
	int charSize;
	ctr_object* arr;
	ctr_argument* newArgumentList;
	arr = ctr_array_new(CtrStdArray, NULL);
	newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	i = 0;
	while( i < myself->value.svalue->vlen ) {
		charSize = ctr_utf8size( *(myself->value.svalue->value + i) );
		newArgumentList->object = ctr_build_string( myself->value.svalue->value + i, charSize );
		ctr_array_push( arr, newArgumentList );
		i += charSize;
	}
	ctr_heap_free( newArgumentList );
	return arr;
}

/**
 * @def
 * [ String ] compare: [ String ]
 *
 *
 * @test484
 */

ctr_object* ctr_string_compare( ctr_object* myself, ctr_argument* argumentList ) {
	argumentList->object = ctr_internal_cast2string(argumentList->object);
	ctr_size maxlen;
	int s, c;
	if (myself->value.svalue->vlen < argumentList->object->value.svalue->vlen) {
		maxlen = myself->value.svalue->vlen;
	} else {
		maxlen = argumentList->object->value.svalue->vlen;
	}
	s = (ctr_number) strncmp(
		myself->value.svalue->value,
		ctr_internal_cast2string(argumentList->object)->value.svalue->value,
		maxlen
	);
	//implementations can differ, only the sign matters -1 0 +1.
	c = 0;
	if (s < 0) c = -1;
	if (s > 0) c = 1;
	return ctr_build_number_from_float( c );
}

/**
 * @def
 * [ String ] < [ String ]
 * 
 *
 * @test485
 */

ctr_object* ctr_string_before(ctr_object* myself, ctr_argument* argumentList ) {
	if ( ctr_string_compare( myself, argumentList )->value.nvalue < 0 ) {
		return ctr_build_bool( 1 );
	}
	return ctr_build_bool( 0 );
}


/**
 * @def
 * [ String ] ≤ [ String ]
 * 
 *
 * @test486
 */

ctr_object* ctr_string_before_or_same(ctr_object* myself, ctr_argument* argumentList ) {
	if ( ctr_string_compare( myself, argumentList )->value.nvalue <= 0 ) {
		return ctr_build_bool( 1 );
	}
	return ctr_build_bool( 0 );
}

/**
 * @def
 * [ String ] > [ String ]
 * 
 *
 * @test487
 */

ctr_object* ctr_string_after(ctr_object* myself, ctr_argument* argumentList ) {
	if ( ctr_string_compare( myself, argumentList )->value.nvalue > 0 ) {
		return ctr_build_bool( 1 );
	}
	return ctr_build_bool( 0 );
}


 /**
 * @def
 * [ String ] ≥ [ String ]
 * 
 *
 * @test488
 */

ctr_object* ctr_string_after_or_same(ctr_object* myself, ctr_argument* argumentList ) {
	if ( ctr_string_compare( myself, argumentList )->value.nvalue >= 0 ) {
		return ctr_build_bool( 1 );
	}
	return ctr_build_bool( 0 );
}

/**
 * @def
 * [ String ] hash: [ String ]
 *
 *
 * @test489
 */

ctr_object* ctr_string_hash_with_key( ctr_object* myself, ctr_argument* argumentList ) {
	char* keyString = ctr_heap_allocate_cstring( ctr_internal_cast2string( argumentList->object ) );
	if ( strlen( keyString ) < 16 ) {
		ctr_heap_free( keyString );
		CtrStdFlow = ctr_error( CTR_ERR_SIPHKEY, 0 );
		return CtrStdNil;
	}
	uint64_t t = siphash24( myself->value.svalue->value, myself->value.svalue->vlen, keyString);
	char* dest = ctr_heap_allocate( 40 );
	snprintf( dest, 40, "%" PRIu64, t );
	ctr_object* hash = ctr_build_string_from_cstring( dest );
	ctr_heap_free( dest );
	ctr_heap_free( keyString );
	return hash;
}

/**
 * @def
 * Code
 *
 *
 * @test490
 */

ctr_object* ctr_build_block(ctr_tnode* node) {
	ctr_object* codeBlockObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTBLOCK);
	codeBlockObject->value.block = node;
	codeBlockObject->link = CtrStdBlock;
	return codeBlockObject;
}

/**
 * @def
 * [ Code ] run
 * 
 *
 * @test491
 */

ctr_object* ctr_block_run(ctr_object* myself, ctr_argument* argList, ctr_object* my) {
	ctr_object* result;
	ctr_tnode* node = myself->value.block;
	if (node == NULL) return CtrStdNil;
	ctr_tlistitem* codeBlockParts = node->nodes;
	ctr_tnode* codeBlockPart1 = codeBlockParts->node;
	ctr_tnode* codeBlockPart2 = codeBlockParts->next->node;
	ctr_tlistitem* parameterList = codeBlockPart1->nodes;
	ctr_tnode* parameter;
	ctr_object* a;
	int sticky;
	ctr_open_context();
	if (parameterList && parameterList->node) {
		parameter = parameterList->node;
		while(argList != NULL) {
			if (parameter && argList->object) {
				a = argList->object;
				ctr_assign_value_to_local(ctr_build_string(parameter->value, parameter->vlen), a);
			}
			if (!argList->next) break;
			argList = argList->next;
			if (!parameterList->next) break;
			parameterList = parameterList->next;
			parameter = parameterList->node;
		}
	}
	if (my) {
		/* me should always point to object, otherwise you have to store me in self and cant use in if */
		ctr_assign_value_to_local(ctr_build_string_from_cstring( ctr_clex_keyword_me_icon ), my );
		ctr_assign_value_to_local(ctr_build_string_from_cstring( ctr_clex_keyword_my_icon ), my );
	}
	ctr_assign_value_to_local(ctr_build_string_from_cstring( CTR_DICT_CURRENT_TASK ), myself ); /* otherwise running block may get gc'ed. */
	ctr_cwlk_subprogram++;
	result = ctr_cwlk_run(codeBlockPart2);
	ctr_cwlk_subprogram--;
	if (result == NULL) {
		if (my) result = my; else result = myself;
	}
	ctr_close_context();
	/* assign result to lower context to prevent it from being GC'ed. */
	if (ctr_in_message) {
		ctr_gc_internal_pin( result );
	}
	if (CtrStdFlow != NULL && CtrStdFlow != CtrStdBreak && CtrStdFlow != CtrStdContinue) {
		ctr_object* catchBlock = ctr_internal_create_object( CTR_OBJECT_TYPE_OTBLOCK );
		catchBlock = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "catch" ), 0);
		if (catchBlock != NULL) {
			ctr_argument* a = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
			a->object = ctr_internal_cast2string(CtrStdFlow);
			CtrStdFlow = NULL;
			ctr_callstack_index -= errstack;
			errstack = 0;
			sticky = a->object->info.sticky;
			a->object->info.sticky = 1;
			ctr_gc_internal_pin( a->object );
			ctr_block_run(catchBlock, a, NULL);
			a->object->info.sticky = sticky;
			ctr_heap_free( a );
			result = myself;
		}
	}
	return result;
}

/**
 * @def
 * [ Code ] while: [ Code ]
 *
 *
 * @test492
 */

ctr_object* ctr_block_while_true(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* block = argumentList->object;
	if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
		CtrStdFlow = ctr_error( CTR_ERR_EXP_BLK, 0 );
	}
	int sticky1, sticky2;
	sticky1 = myself->info.sticky;
	sticky2 = argumentList->object->info.sticky;
	myself->info.sticky = 1;
	argumentList->object->info.sticky = 1;
	while (1 && !CtrStdFlow) {
		ctr_object* result = ctr_internal_cast2bool(ctr_block_run(block, argumentList, NULL));
		if (result->value.bvalue == 0 || CtrStdFlow) break;
		ctr_block_run(myself, argumentList, NULL);
		if (CtrStdFlow == CtrStdContinue) CtrStdFlow = NULL; /* consume continue */
	}
	if (CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL; /* consume break */
	myself->info.sticky = sticky1;
	argumentList->object->info.sticky = sticky2;
	return myself;
}

ctr_object* ctr_block_runIt(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* result;
	result = ctr_block_run(myself, argumentList, myself); /* here me/my refers to block itself not object - this allows closures. */
	if (CtrStdFlow == CtrStdBreak || CtrStdFlow == CtrStdContinue) CtrStdFlow = NULL; /* consume break */
	return result;
}

/**
 * @def
 * [ Code ] set: [ String ] value: [ Object ]
 *
 *
 * @test493
 */

ctr_object* ctr_block_set(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* key = ctr_internal_cast2string(argumentList->object);
	ctr_object* value = argumentList->next->object;
	ctr_internal_object_set_property(myself, key, value, 0);
	return myself;
}

/**
 * @def
 * [ Code ] error: [ Object ]
 *
 *
 * @test494
 */

ctr_object* ctr_block_error(ctr_object* myself, ctr_argument* argumentList) {
	CtrStdFlow = argumentList->object;
	CtrStdFlow->info.sticky = 1;
	return myself;
}

/**
 * @def
 * [ Code ] catch: [ Code ]
 *
 *
 * @test495
 */

ctr_object* ctr_block_catch(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* catchBlock = argumentList->object;
	ctr_internal_object_delete_property(myself, ctr_build_string_from_cstring( "catch" ), 0 );
	ctr_internal_object_add_property(myself, ctr_build_string_from_cstring( "catch" ), catchBlock, 0 );
	return myself;
}

/**
 * @def
 * [ Code ] string
 *
 *
 * @test496
 */

ctr_object* ctr_block_to_string(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_string_from_cstring( CTR_DICT_TASK );
}
