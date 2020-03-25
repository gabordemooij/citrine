#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <regex.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "citrine.h"
#include "siphash.h"

ctr_size ctr_program_length;
uint64_t    ctr_cwlk_subprogram;
int ctr_in_message;

/**
 * Format a number to the target language.
 */
char* ctr_international_number(char* old_number, char* new_number) {
	char i, j, l, s, k, old_length;
	char* x;
	j = 0;
	k = 0;
	l = strlen(CTR_DICT_NUM_DEC_SEP);
	old_length = strlen( old_number );
	x = strchr( old_number, '.' );
	if ( x == NULL ) {
		s = old_length;
	} else {
		s = (char) (x - old_number);
	}
	for( i = 0; i < old_length; i ++ ) {
		if ( *(old_number + i) == '.' ) {
			strncpy( new_number + j , CTR_DICT_NUM_DEC_SEP, l);
			j += l;
			continue;
		}
		if ( (i < s) && ( k > 0 ) && ( ( s - i ) % 3 ) == 0 ) {
			strncpy( new_number + j, CTR_DICT_NUM_THO_SEP, ctr_clex_keyword_num_sep_tho_len );
			j += ctr_clex_keyword_num_sep_tho_len;
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
 * Nil
 *
 * Nil represents 'nothing' or NULL in other languages.
 * Any object property that has not been assigned a value
 * will contain Nil. Unlike some other programming languages
 * Citrine has no concept of 'undefined' or isset, Nil is actually the
 * same as 'undefined' or not set.
 *
 * Literal:
 *
 * Nil
 *
 * In other languages:
 * Dutch: Niets
 */
ctr_object* ctr_build_nil() {
	return CtrStdNil;
}

/**
 * [Nil] Nil?
 *
 * Nil always answers this message with a boolean object 'True'.
 *
 * In other languages:
 * Dutch: [Niets] Niets? | antwoord altijd met Waar.
 */
ctr_object* ctr_nil_is_nil(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(1);
}

/**
 * [Nil] string
 *
 * Returns the string representation of Nil: 'Nil'.
 *
 * In other languages:
 * Dutch: [Niets] tekst | geeft de tekstweergave van Niets (en dat is altijd 'Niets')
 */
ctr_object* ctr_nil_to_string(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_string_from_cstring( CTR_DICT_NIL );
}

/**
 * [Nil] number
 *
 * Returns the numerical representation of Nil: 0.
 *
 * In other languages:
 * Dutch: [Niets] boolean | geeft de getalswaarde voor Niets terug (altijd 0).
 */
ctr_object* ctr_nil_to_number(ctr_object* myself, ctr_argument* ctr_argumentList) {
	return ctr_build_number_from_float(0);
}

/**
 * [Nil] boolean
 *
 * Returns the boolean representation of Nil: False.
 *
 * In other languages:
 * Dutch: [Niets] boolean | Niets is altijd gelijk aan Onwaar
 */
ctr_object* ctr_nil_to_boolean(ctr_object* myself, ctr_argument* ctr_argumentList) {
	return ctr_build_bool(0);
}

/**
 * Object
 *
 * This is the base object, the parent of all other objects.
 * It contains essential object oriented programming features.
 *
 * In other languages:
 * Dutch: Object | Dit is het generieke object, de vader van alle objecten
 */
ctr_object* ctr_object_make(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* objectInstance = NULL;
	objectInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	objectInstance->link = myself;
	return objectInstance;
}

/**
 * [Object] type
 *
 * Returns a string representation of the type of object.
 *
 * In other languages:
 * Dutch: [Object] type | Geeft het basistype object terug
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
			return ctr_build_string_from_cstring( CTR_DICT_CODE_BLOCK );
		default:
			return ctr_build_string_from_cstring( CTR_DICT_OBJECT );
	}
}

/**
 * [Object] string
 *
 * Returns a string representation of a generic object.
 * This string representation will be:
 *
 * [Object]
 *
 * In other languages:
 * Dutch: [Object] tekst | Geeft het de tekstuele omschrijving van het moederobject ('[Object]')
 */
ctr_object* ctr_object_to_string( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_build_string_from_cstring( CTR_SYM_OBJECT );
}

/**
 * [Object] number
 *
 * Returns a numerical representation of the object. This basic behavior, part
 * of any object will just return 1. Other objects typically override this
 * behavior with more useful implementations.
 *
 * In other languages:
 * Dutch: [Object] getal | Geeft de getalswaarde van het algemene object (altijd 1)
 */
ctr_object* ctr_object_to_number(ctr_object* myself, ctr_argument* ctr_argumentList) {
	return ctr_build_number_from_float(1);
}

/**
 * [Object] boolean
 *
 * Returns a boolean representation of the object. This basic behavior, part
 * of any object will just return True. Other objects typically override this
 * behavior with more useful implementations.
 *
 * In other languages:
 * Dutch: [Object] boolean | Geeft de waarheidswaarde van het algemene object (altijd Waar)
 */
ctr_object* ctr_object_to_boolean(ctr_object* myself, ctr_argument* ctr_argumentList) {
	return ctr_build_bool(1);
}

/**
 * [Object] equals: [other]
 *
 * Tests whether the current instance is the same as
 * the argument. You also use the message '=' for this
 * however, often that message will be overridden by a
 * derived object (Number will use = to compare the numeric values
 * for instance).
 *
 * Usage:
 *
 * object equals: other
 *
 * In other languages:
 * Dutch: [Object] gelijk: [Object] | Geeft Waar terug als beide objecten een en dezelfde zijn
 */
ctr_object* ctr_object_equals(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherObject = argumentList->object;
	if (otherObject == myself) return ctr_build_bool(1);
	return ctr_build_bool(0);
}

/**
 * [Object] myself
 *
 * Returns the object itself.
 *
 * In other languages:
 * Dutch: [Object] mijzelf | Geeft het object zelf terug
 */
ctr_object* ctr_object_myself(ctr_object* myself, ctr_argument* argumentList) {
	return myself;
}

/**
 * [Object] do
 *
 * Activates 'chain mode'. If chain mode is active, all messages will
 * return the recipient object regardless of their return signature.
 * The 'do' message tells the object to always return itself and disgard
 * the original return value until the message 'done' has been received.
 *
 * Usage:
 *
 * a := List ← 'hello' ; 'world' ; True ; Nil ; 666.
 * a do pop shift prepend: 'hi', append: 999, done.
 *
 * In other languages:
 * Dutch: [Object] doen | Stel object zo in dat het alle berichten antwoord met zichzelf
 */
ctr_object* ctr_object_do( ctr_object* myself, ctr_argument* argumentList ) {
	myself->info.chainMode = 1;
	return myself;
}

/**
 * [Object] done
 *
 * Deactivates 'chain mode'.
 *
 * In other languages:
 * Dutch: [Object] klaar | Stop de doen-modus
 */
ctr_object* ctr_object_done( ctr_object* myself, ctr_argument* argumentList ) {
	myself->info.chainMode = 0;
	return myself;
}

/**
 * [Object] copy
 *
 * Contrary to other languages, all objects, even booleans, numbers and
 * strings are assigned and passed by reference. To create a shallow copy
 * of a number, string or boolean send the message 'copy'.
 *
 * Usage:
 *
 * a := 5.
 * b := a copy.
 * b add: 1.
 *
 * In other languages:
 * Dutch: [Object] kopieer | Geeft een kopie van het object terug
 */
ctr_object* ctr_bool_copy( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_build_bool(myself->value.bvalue);
}

ctr_object* ctr_number_copy( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_build_number_from_float(myself->value.nvalue);
}

ctr_object* ctr_string_copy( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_build_string(myself->value.svalue->value, myself->value.svalue->vlen);
}

/**
 * [Object] case: [Object] do: [Block].
 *
 * This message makes the recipient compare itself to the specified object.
 * If the recipient considers itself to be equal, it will carry out the
 * instructions in the associated block of code. The recipient will send
 * the message '=' to itself with the other object as an argument. This leaves
 * it up to the recipient to determine whether the objects are considered
 * equal. If the recipient decides the objects are not equal, the associated
 * code block will be ignored. Note that this allows you to implement a
 * so-called switch-statement like those found in other languages. Because
 * of the generic implementation, you can use the case statements on almost
 * any object. Case-do statements may provide a readable alternative to a
 * long list of if-else messages.
 *
 * The example program below will print the text 'It's a Merlot!'.
 *
 * Usage:
 *
 * wine
 *	case: 'cabernet' do: { ✎ write: 'it\'s a Cabernet!'. },
 *	case: 'syrah'    do: { ✎ write: 'it\'s a Syrah!'.    },
 *	case: 'merlot'   do: { ✎ write: 'it\'s a Merlot!'.   },
 *	case: 'malbec'   do: { ✎ write: 'it\'s a Malbec!'.   }.
 *
 * In other languages:
 * Dutch: [Object] geval: [Object] doen: [Blok]| Voert het blok bij doen: uit als geval: Waar is
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
			ctr_send_message( myself, "=", 1, compareArguments )
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
 * [Object] message: [String] arguments: [List]
 *
 * Sends a custom or 'dynamic' message to an object. This takes a string containing
 * the message to be send to the object and an array listing the arguments at the
 * correct indexes. If the array fails to provide the correct indexes this will
 * generate an out-of-bounds error coming from the Array object. If something other
 * than an List is provided an error will be thrown as well.
 *
 * Usage:
 *
 * ☞ str := 'write:'.
 * ✎ message: 'write:' arguments: (List ← 'Hello World').
 *
 * This will print the string 'Hello world' on the screen using a dynamically
 * crafted message.
 *
 * In other languages:
 * Dutch: [Object] bericht:[Tekst] argumenten:[Reeks] | Stuurt een dynamisch bericht naar object
 */
ctr_object* ctr_object_message( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_object* message = ctr_internal_cast2string( argumentList->object );
	ctr_object* arr     = argumentList->next->object;
	if ( arr->info.type != CTR_OBJECT_TYPE_OTARRAY ) {
		ctr_error( CTR_ERR_EXP_ARR, 0 );
		return CtrStdNil;
	}
	ctr_size length = (int) ctr_array_count( arr,  NULL )->value.nvalue;
	int i = 0;
	ctr_argument* args = ctr_heap_allocate( sizeof( ctr_argument ) );
	ctr_argument* cur  = args;
	for ( i = 0; i < length; i ++ ) {
		ctr_argument* index = ctr_heap_allocate( sizeof( ctr_argument ) );
		if ( i > 0 ) {
			cur->next = ctr_heap_allocate( sizeof( ctr_argument ) );
			cur = cur->next;
		}
		index->object = ctr_build_number_from_float( (double) i + 1 );
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
 * [Object] on: [String] do: [Block]
 *
 * Makes the object respond to a new kind of message.
 * Use the semicolons to indicate the positions of the arguments to be
 * passed.
 *
 * Usage:
 *
 * object on: 'greet' do: { ... }.
 * object on: 'between:and:' do: { ... }.
 *
 * In other languages:
 * Dutch: [Object] bij: [Tekst] doen:[Codeblok] | Voegt gedrag toe aan object (bij ontvangst bericht - doen)
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
 * [Object] respond: [String]
 *
 * Variations:
 *
 * [Object] respond: [String] and: [String]
 * [Object] respond: [String] and: [String] and: [String]
 * [Object] respond: [String] and: [String] and: [String] and: [String]
 *
 * Default respond-to implemention, does nothing.
 * You can override this behaviour to implement generic behaviour.
 * Listening to these messages allows users to send any message to an
 * object. For instance an object can respond to any message it does not
 * understand by echoing the message.
 *
 * In other languages:
 * Dutch: [Object] reageer: [Tekst] en: [Tekst] en: [Tekst]
 */
ctr_object* ctr_object_respond(ctr_object* myself, ctr_argument* argumentList) {
	return myself;
}


ctr_object* ctr_object_respond_and(ctr_object* myself, ctr_argument* argumentList) {
	return myself;
}

ctr_object* ctr_object_respond_and_and(ctr_object* myself, ctr_argument* argumentList) {
	return myself;
}

ctr_object* ctr_object_respond_and_and_and(ctr_object* myself, ctr_argument* argumentList) {
	return myself;
}

/**
 * [Object] Nil?
 *
 * Default Nil? implementation.
 *
 * Always returns boolean object False.
 *
 * In other languages:
 * Dutch: [Object] Niets? | Vraagt aan een object of het niets is (Object antwoord altijd Onwaar)
 */
ctr_object* ctr_object_is_nil(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(0);
}

/**
 * [Object] learn: [String] means: [String].
 *
 * Teaches any object to repsond to the first specified message just like
 * it would upon receiving the second. This allows you to map existing
 * responses to new messages. You can use this to translate messages into your native
 * language. After mapping, sending the alias message will be just as fast
 * as sending the original message. You can use this to create programs
 * in your native language without sacrficing performance. Of course the mapping itself
 * has a cost, but the mapped calls will be 'toll-free'.
 *
 * Usage:
 *
 * Boolean learn: 'yes:' means: 'true:'.
 *
 * In other languages:
 * Dutch: [Object] leer: [Tekst] betekent: [Tekst] | Leert object dat bericht 1 hetzelfde betekent als bericht 2
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
 * Boolean
 *
 * Literal:
 *
 * True
 * False
 *
 * In other languages:
 * Dutch: Boolean (waarheid: Waar of Onwaar)
 */
ctr_object* ctr_build_bool(int truth) {
	ctr_object* boolObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTBOOL);
	if (truth) boolObject->value.bvalue = 1; else boolObject->value.bvalue = 0;
	boolObject->info.type = CTR_OBJECT_TYPE_OTBOOL;
	boolObject->link = CtrStdBool;
	return boolObject;
}

/**
 * [Boolean] = [other]
 *
 * Tests whether the other object (as a boolean) has the
 * same value (boolean state True or False) as the current one.
 *
 * Usage:
 *
 * (True = False) false: { ✎ write: 'This is not True!'. }.
 *
 * In other languages:
 * Dutch: Boolean (waarheid: Waar of Onwaar)
 */
ctr_object* ctr_bool_eq(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(ctr_internal_cast2bool(argumentList->object)->value.bvalue == myself->value.bvalue);
}


/**
 * [Boolean] != [other]
 *
 * Tests whether the other object (as a boolean) has the
 * same value (boolean state True or False) as the current one.
 *
 * Usage:
 *
 * (True != False) true: { ✎ write: 'This is not True!'. }.
 */
ctr_object* ctr_bool_neq(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(ctr_internal_cast2bool(argumentList->object)->value.bvalue != myself->value.bvalue);
}

/**
 * [Boolean] string
 *
 * Returns a string representation of a boolean value, i.e. 'True' or 'False'.
 *
 * In other languages:
 * Dutch: tekst | geeft beschrijving van de waarheidswaarde ('Waar' of 'Onwaar')
 */
ctr_object* ctr_bool_to_string(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.bvalue == 1) {
		return ctr_build_string_from_cstring( CTR_DICT_TRUE );
	} else {
		return ctr_build_string_from_cstring( CTR_DICT_FALSE );
	}
}

/**
 * [Boolean] break
 *
 * Breaks out of the current block and bubbles up to the parent block if
 * the value of the receiver equals boolean True.
 *
 * Usage:
 *
 * { :iteration
 *     (iteration > 10) break.
 * } * 20.
 *
 * In other languages:
 * Dutch: [Boolean] afbreken | Indien waarheidswaarde gelijk is aan Waar dan wordt de lus afgebroken
 */
ctr_object* ctr_bool_break(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.bvalue) {
		CtrStdFlow = CtrStdBreak; /* If error = Break it's a break, there is no real error. */
	}
	return myself;
}

/**
 * [Boolean] continue
 *
 * Skips the remainder of the current block in a loop, continues to the next
 * iteration.
 *
 * Usage:
 *
 * (iteration > 10) continue.
 *
 * In other languages:
 * Dutch: [Boolean] doorgaan | Indien verzonden naar Waar, slaat de rest van het blok over en vervolgt de lus
 */
ctr_object* ctr_bool_continue(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.bvalue) {
		CtrStdFlow = CtrStdContinue; /* If error = Continue, then it breaks only one iteration (return). */
	}
	return myself;
}

/**
 * [Boolean] true: [block]
 *
 * Executes a block of code if the value of the boolean
 * object is True.
 *
 * Usage:
 *
 * (some expression) true: { ... }.
 *
 * In other languages:
 * Dutch: [Boolean] waar: [Codeblok] |  Indien verzonden naar Waar, voert het gegeven blok uit
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
 * [Boolean] false: [block]
 *
 * Executes a block of code if the value of the boolean
 * object is True.
 *
 * Usage:
 *
 * (some expression) false: { ... }.
 *
 * In other languages:
 * Dutch: [Boolean] onwaar: [Codeblok] |  Indien verzonden naar Onwaar, voert het gegeven blok uit
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
 * [Boolean] not
 *
 * Returns the opposite of the current value.
 *
 * Usage:
 * True := False not.
 *
 * In other languages:
 * Dutch: [Boolean] niet | Geeft de omgekeerde waarde terug
 */
ctr_object* ctr_bool_not(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(!myself->value.bvalue);
}

/**
 * [Boolean] either: [this] or: [that]
 *
 * Returns argument #1 if boolean value is True and argument #2 otherwise.
 *
 * In other languages:
 * Dutch: [Boolean] of: [Object] of: [Object] | Antwoord met 1e object als Waar anders 2e.
 */
ctr_object* ctr_bool_either_or(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.bvalue) {
		return argumentList->object;
	} else {
		return argumentList->next->object;
	}
}

/**
 * [Boolean] and: [other]
 *
 * Returns True if both the object value is True and the
 * argument is True as well.
 *
 * Usage:
 *
 * a and: b
 *
 * In other languages:
 * Dutch: [Boolean] en: [Object] | Antwoord met Waar als beide Waar zijn
 */
ctr_object* ctr_bool_and(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* other = ctr_internal_cast2bool(argumentList->object);
	return ctr_build_bool((myself->value.bvalue && other->value.bvalue));
}

/**
 * [Boolean] nor: [other]
 *
 * Returns True if the object value is False and the
 * argument is False as well.
 *
 * Usage:
 *
 * a nor: b
 *
 * In other languages:
 * Dutch: [Boolean] noch: [Object] | Antwoord met Waar als beide Onwaar zijn
 */
ctr_object* ctr_bool_nor(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* other = ctr_internal_cast2bool(argumentList->object);
	return ctr_build_bool((!myself->value.bvalue && !other->value.bvalue));
}

/**
 * [Boolean] or: [other]
 *
 * Returns True if either the object value is True or the
 * argument is True or both are True.
 *
 * Usage:
 *
 * a or: b
 *
 * In other languages:
 * Dutch: [Boolean] of: [Object] | Antwoord met Waar als een van beide objecten Waar is
 */
ctr_object* ctr_bool_or(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* other = ctr_internal_cast2bool(argumentList->object);
	return ctr_build_bool((myself->value.bvalue || other->value.bvalue));
}

/**
 * [Boolean] number
 *
 * Returns 0 if boolean is False and 1 otherwise.
 *
 * In other languages:
 * Dutch: [Boolean] getal | Antwoord 0 als Onwaar en 1 als Waar
 */
ctr_object* ctr_bool_to_number(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float( (ctr_number) myself->value.bvalue );
}

/**
 * Number
 *
 * Literal:
 *
 * 0
 * 1
 * -8
 * 2.5
 *
 * Represents a number object in Citrine.
 *
 * In other languages:
 * Dutch: Getal, representeert een getal
 */
ctr_object* ctr_build_number(char* n) {
	ctr_object* numberObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNUMBER);
	numberObject->value.nvalue = atof(n);
	numberObject->link = CtrStdNumber;
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
 * [Number] > [other]
 *
 * Upon receiving this binary message, the Number object will
 * compare itself to the specified Number (other). If the value of
 * the Number object is higher than the other Number object, it will
 * answer with a boolean object True,
 * otherwise it will answer with a boolean object False.
 *
 * Usage:
 *
 * ☞ x := 8 > 7.
 *
 * The code snippet above will compare the two number objects.
 * The result (True) will be stored in variable x.
 *
 * In other languages:
 * Dutch: [Getal] > [Getal] | Antwoord Waar als eerste getal groter is dan het tweede
 */
ctr_object* ctr_number_higherThan(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue > otherNum->value.nvalue));
}

/**
 * [Number] ≥ [other]
 *
 * Upon receiving this binary message, the Number object will
 * compare itself to the specified Number (other). If the value of
 * the Number object is higher than or equal to the other Number object, it will
 * answer with a boolean object True,
 * otherwise it will answer with a boolean object False.
 *
 * Usage:
 *
 * ☞ x := 8 ≥ 7.
 *
 * The code snippet above will compare the two number objects.
 * The result (True) will be stored in variable x.
 *
 * In other languages:
 * Dutch: [Getal] ≥ [Getal] | Antwoord Waar als eerste getal groter is dan of gelijk is aan het tweede
 */
ctr_object* ctr_number_higherEqThan(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue >= otherNum->value.nvalue));
}

/**
 * [Number] < [other]
 *
 * Upon receiving this binary message, the Number object will
 * compare itself to the specified Number (other). If the value of
 * the Number object is less than the other Number object, it will
 * answer with a boolean object True,
 * otherwise it will answer with a boolean object False.
 *
 * Usage:
 *
 * ☞ x := 7 < 8.
 *
 * The code snippet above will compare the two number objects.
 * The result (True) will be stored in variable x.
 *
 * In other languages:
 * Dutch: [Getal] < [Getal] | Antwoord Waar als eerste getal kleiner is dan het tweede
 */
ctr_object* ctr_number_lowerThan(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue < otherNum->value.nvalue));
}

/**
 * [Number] ≤ [other]
 *
 * Upon receiving this binary message, the Number object will
 * compare itself to the specified Number (other). If the value of
 * the Number object is lower than or equal to the other Number object, it will
 * answer with a boolean object True,
 * otherwise it will answer with a boolean object False.
 *
 * Usage:
 *
 * ☞ x := 7 ≤ 8.
 *
 * The code snippet above will compare the two number objects.
 * The result (True) will be stored in variable x.
 *
 * In other languages:
 * Dutch: [Getal] ≤ [Getal] | Antwoord Waar als eerste getal kleiner of gelijk is dan het tweede
 */
ctr_object* ctr_number_lowerEqThan(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue <= otherNum->value.nvalue));
}

/**
 * [Number] = [other]
 *
 * Upon receiving this binary message, the Number object will
 * compare itself to the specified Number (other). If the value of
 * the Number object equals the other Number object, it will
 * answer with a boolean object True,
 * otherwise it will answer with a boolean object False.
 *
 * Usage:
 *
 * ☞ x := 8 = 8.
 *
 * The code snippet above will compare the two number objects.
 * The result (True) will be stored in variable x.
 *
 * In other languages:
 * Dutch: [Getal] = [Getal] | Antwoord Waar als eerste gelijk is aan het tweede
 */
ctr_object* ctr_number_eq(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool(myself->value.nvalue == otherNum->value.nvalue);
}

/**
 * [Number] = [other]
 *
 * Upon receiving this binary message, the Number object will
 * compare itself to the specified Number (other). If the value of
 * the Number object does not equal the other Number object, it will
 * answer with a boolean object True,
 * otherwise it will answer with a boolean object False.
 *
 * Usage:
 *
 * ☞ x := 7 ≠ 8.
 *
 * The code snippet above will compare the two number objects.
 * The result (True) will be stored in variable x.
 *
 * In other languages:
 * Dutch: [Getal] ≠ [Getal] | Antwoord Waar als eerste ongelijk is aan het tweede
 */
ctr_object* ctr_number_neq(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool(myself->value.nvalue != otherNum->value.nvalue);
}

/**
 * [Number] between: [Number] and: [Number]
 *
 * Returns a random number between the specified boundaries,
 * including the upper and lower boundary of the number. So,
 * asking for a number between 0 and 10 may result in numbers like
 * 0 and 10 as well. Only rounded numbers are returned and the
 * boundaries will be rounded as well. So a random number between
 * 0.5 and 1 will always result in 1. Negative numbers are allowed
 * as well.
 *
 * Usage:
 *
 * ☞ x := Number between 0 and: 10.
 *
 * In other languages:
 * Dutch: [Getal] tussen: [Getal] en: [Getal] | Geeft een getal tussen getal1 en 2.
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
 * [Number] odd?
 *
 * Returns True if the number is odd and False otherwise.
 *
 * In other languages:
 * Dutch: [Getal] oneven | Antwoord Waar als het getal oneven is.
 */
ctr_object* ctr_number_odd(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool((int)myself->value.nvalue % 2);
}

/**
 * [Number] even?
 *
 * Returns True if the number is even and False otherwise.
 *
 * In other languages:
 * Dutch: [Getal] even | Antwoord Waar als het getal even is.
 */
ctr_object* ctr_number_even(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(!((int)myself->value.nvalue % 2));
}

/**
 * [Number] + [Number]
 *
 * Adds the other number to the current one. Returns a new
 * number object.
 *
 * In other languages:
 * Dutch: [Getal] + [Getal] | Geeft de som der getallen.
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
 * [Number] add: [Number]
 *
 * Increases the number ITSELF by the specified amount, this message will change the
 * value of the number object itself instead of returning a new number.
 *
 * In other languages:
 * Dutch: [Getal] optellen: [Getal] | Telt het gespecificeerde getal op bij het huidige.
 */
ctr_object* ctr_number_inc(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	myself->value.nvalue += otherNum->value.nvalue;
	return myself;
}

/**
 * [Number] - [Number]
 *
 * Subtracts the other number from the current one. Returns a new
 * number object.
 *
 * In other languages:
 * Dutch: [Getal] - [Getal] | Trekt 2 getallen van elkaar af.
 */
ctr_object* ctr_number_minus(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_number a = myself->value.nvalue;
	ctr_number b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a-b));
}

/**
 * [Number] subtract: [number]
 *
 * Decreases the number ITSELF by the specified amount, this message will change the
 * value of the number object itself instead of returning a new number.
 *
 * In other languages:
 * Dutch: [Getal] aftrekken: [Getal] | Trekt het gespecificeerde getal af van het huidige.
 */
ctr_object* ctr_number_dec(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	myself->value.nvalue -= otherNum->value.nvalue;
	return myself;
}

/**
 * [Number] * [Number or Block]
 *
 * Multiplies the number by the specified multiplier. Returns a new
 * number object.
 *
 * In other languages:
 * Dutch: [Getal] * [Getal] | Geeft het product van de getallen.
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
 * [Block] * [Number]
 *
 * Runs the block of code a 'Number' of times.
 * This is the most basic form of a loop.
 * The example runs the block 7 times. The current iteration
 * number is passed to the block as a parameter (i in this example).
 *
 * Usage:
 *
 * { :i ✎ write: i. } * 7.
 *
 * In other languages:
 * Dutch: [Codeblok] * [Getal] | Voert het blok het opgegeven aantal keren uit.
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
		indexNumber = ctr_build_number_from_float((ctr_number) i + 1);
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
 * [Number] multiply by: [Number]
 *
 * Multiplies the number ITSELF by multiplier, this message will change the
 * value of the number object itself instead of returning a new number.
 * Use this message to apply the operation to the object itself instead
 * of creating and returning a new object.
 *
 * Usage:
 *
 * x := 5.
 * x multiply by: 2.
 *
 * In other languages:
 * Dutch: [Getal] vermenigvuldig met: [Getal] | Vermenigvuldigd het huidige getal met de opgegeven waarde.
 */
ctr_object* ctr_number_mul(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	myself->value.nvalue *= otherNum->value.nvalue;
	return myself;
}

/**
 * [Number] / [Number]
 *
 * Divides the number by the specified divider. Returns a new
 * number object.
 *
 * In other languages:
 * Dutch: [Getal] / [Getal] | Deling.
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
 * [Number] devide by: [Number]
 *
 * Divides the number ITSELF by divider, this message will change the
 * value of the number object itself instead of returning a new number.
 * Use this message to apply the operation to the object itself instead
 * of generating a new object.
 *
 * Usage:
 *
 * x := 10.
 * x divide by: 2.
 *
 * In other languages:
 * Dutch: [Getal] deel door: [Getal] | Deelt het huidige getal door de opgegeven waarde.
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
 * [Number] modulo: [modulo]
 *
 * Returns the modulo of the number. This message will return a new
 * object representing the modulo of the recipient.
 *
 * Usage:
 *
 * x := 11 modulo: 3.
 *
 * Use this message to apply the operation of division to the
 * object itself instead of generating a new one.
 *
 * In other languages:
 * Dutch: [Getal] modulo: [Getal]
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
 * [Number] power: [power]
 *
 * Returns a new object representing the
 * number to the specified power.
 * The example above will raise 2 to the power of 8 resulting in
 * a new Number object: 256.
 *
 * Usage:
 *
 * x := 2 power: 8.
 *
 * In other languages:
 * Dutch: [Getal] tot de macht: [Getal] | Machtsverheffen.
 */
ctr_object* ctr_number_pow(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_number a = myself->value.nvalue;
	ctr_number b = otherNum->value.nvalue;
	return ctr_build_number_from_float(pow(a,b));
}

/**
 * [Number] positive?
 *
 * Returns a boolean indicating wether the number is positive.
 * This message will return a boolean object 'True' if the recipient is
 * positive and 'False' otherwise.
 * The example above will print the message because hope is higher than 0.
 *
 * Usage:
 *
 * hope := 0.1.
 * ( hope positive? ) true: {
 *     ✎ write: 'Still a little hope for humanity'.
 * }.
 *
 * In other languages:
 * Dutch: [Getal] positief? | Antwoord Waar als het getal groter is dan 0.
 */
ctr_object* ctr_number_positive(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool( ( myself->value.nvalue > 0) );
}

/**
 * [Number] negative?
 *
 * Returns a boolean indicating wether the number is negative.
 * This message will return a boolean object 'True' if the recipient is
 * negative and 'False' otherwise. It's the eaxct opposite of the 'positive'
 * message.
 * The example above will print the message because the value of the variable
 * hope is less than 0.
 *
 * Usage:
 *
 * hope := -1.
 * (hope negative?) ifTrue: { Pen write: 'No hope left'. }.
 *
 * In other languages:
 * Dutch: [Getal] negatief? | Antwoord Waar als het getal kleiner is dan 0.
 */
ctr_object* ctr_number_negative(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool( ( myself->value.nvalue < 0) );
}

/**
 * [Number] floor
 *
 * Gives the largest integer less than the recipient.
 * The example above applies the floor function to the recipient (4.5)
 * returning a new number object (4).
 *
 * Usage:
 *
 * x := 4.5
 * y := x floor.
 *
 * In other languages:
 * Dutch: [Getal]  afgerond naar beneden
 */
ctr_object* ctr_number_floor(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(floor(myself->value.nvalue));
}

/**
 * [Number] qualify: 'meters'.
 *
 * Qualifies a number. Alias for: [Number] [String].
 * See the [Number] [String] message signature for more details.
 */

/**
 * [Number] [String]
 *
 * Qualifies a number. By sending an arbitrary (undocumented) unary message to
 * a Number object your message will be set as the qualification property of the number
 * and passed around along with the number value itself.
 *
 * Usage:
 *
 * Number learn: 'plus:' means: '+'.
 * Number on: '+' do: { :x
 *     ☞ rate := 1.
 *     ☞ currency := x qualification.
 *     (currency = 'euros') true: {
 *         rate := 2.
 *     }.
 *     ↲ (⛏ plus: (x * rate)).
 * }.
 * ☞ money := 3 dollars + 2 euros.
 */
ctr_object* ctr_number_qualify(ctr_object* myself, ctr_argument* argumentList) {
	ctr_internal_object_set_property( myself, ctr_build_string_from_cstring( CTR_DICT_QUALIFICATION ), ctr_internal_cast2string( argumentList->object ), CTR_CATEGORY_PRIVATE_PROPERTY );
	return myself;
}

/**
 * [Number] qualification.
 *
 * Returns the qualification of a number object. For instance, as a
 * number (let's say 99) has been qualified as the number of bottles using
 * a message like: '99 bottles' this message will return the descriptive string
 * 'bottles'. For usage examples, please consult the [Number] [String] message
 * signature.
 */
ctr_object* ctr_number_qualification(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* answer = ctr_internal_object_find_property( myself, ctr_build_string_from_cstring( CTR_DICT_QUALIFICATION ), CTR_CATEGORY_PRIVATE_PROPERTY );
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
 * [Number] ceil
 *
 * Rounds up the recipient number and returns the next higher integer number
 * as a result.
 * The example above applies the ceiling function to the recipient (4.5)
 * returning a new number object (5).
 *
 * Usage:
 *
 * x := 4.5.
 * y = x ceil.
 *
 * In other languages:
 * Dutch: [Getal]  afgerond naar boven
 */
ctr_object* ctr_number_ceil(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(ceil(myself->value.nvalue));
}

/**
 * [Number] round
 *
 * Returns the rounded number.
 */
ctr_object* ctr_number_round(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(round(myself->value.nvalue));
}

/**
 * [Number] absolute
 *
 * Returns the absolute (unsigned, positive) value of the number.
 * The example above strips the sign off the value -7 resulting
 * in 7.
 *
 * Usage:
 *
 * x := -7.
 * y := x absolute.
 *
 * In other languages:
 * Dutch: [Getal] absoluut
 */
ctr_object* ctr_number_abs(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(fabs(myself->value.nvalue));
}

/**
 * [Number] square root
 *
 * Returns the square root of the recipient.
 * The example above takes the square root of 49, resulting in the
 * number 7.
 *
 * Usage:
 *
 * ☞ x := 49.
 * ☞ y := x square root.
 *
 * In other languages:
 * Dutch: [Getal] vierkantswortel
 */
ctr_object* ctr_number_sqrt(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(sqrt(myself->value.nvalue));
}

/**
 * [Number] byte
 *
 * Converts a number to a single byte.
 *
 * In other languages:
 * Dutch: [Getal] byte
 */
ctr_object* ctr_number_to_byte(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* str = ctr_build_empty_string();
	str->value.svalue->value = ctr_heap_allocate( 1 );
	str->value.svalue->vlen = 1;
	*(str->value.svalue->value) = (uint8_t) myself->value.nvalue;
	return str;
}

/**
 * [Number] string
 *
 * Wrapper for cast function.
 *
 * In other languages:
 * Dutch: [Getal] tekst
 */
char* ctr_international_number(char* old_number, char* new_number);
ctr_object* ctr_number_to_string(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* o = myself;
	int slen;
	char* s;
	char* q;
	char* p;
	char* buf;
	int bufSize;
	ctr_object* stringObject;
	s = ctr_heap_allocate( 100 * sizeof( char ) );
	q = ctr_heap_allocate( 100 * sizeof( char ) );
	bufSize = 100 * sizeof( char );
	buf = ctr_heap_allocate( bufSize );
	snprintf( buf, 99, "%.10f", o->value.nvalue );
	p = buf + strlen(buf) - 1;
	while ( *p == '0' && *p-- != '.' );
	*( p + 1 ) = '\0';
	if ( *p == '.' ) *p = '\0';
	strncpy( s, buf, strlen( buf ) );
	ctr_heap_free( buf );
	q = ctr_international_number( s, q );
	slen = strlen(q);
	stringObject = ctr_build_string(q, slen);
	ctr_heap_free( q );
	ctr_heap_free( s );
	return stringObject;
}

/**
 * [Number] international number
 *
 * Returns a string representation of the number, parsing the number
 * as an international number (using the dot as the decimal separator and
 * using no thousands separator).
 */
ctr_object* ctr_number_to_string_flat(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* o = myself;
	int slen;
	char* s;
	char* p;
	char* buf;
	int bufSize;
	ctr_object* stringObject;
	s = ctr_heap_allocate( 100 * sizeof( char ) );
	bufSize = 100 * sizeof( char );
	buf = ctr_heap_allocate( bufSize );
	snprintf( buf, 99, "%.10f", o->value.nvalue );
	p = buf + strlen(buf) - 1;
	while ( *p == '0' && *p-- != '.' );
	*( p + 1 ) = '\0';
	if ( *p == '.' ) *p = '\0';
	strncpy( s, buf, strlen( buf ) );
	ctr_heap_free( buf );
	slen = strlen(s);
	stringObject = ctr_build_string(s, slen);
	ctr_heap_free( s );
	return stringObject;
}

/**
 * [Number] boolean
 *
 * Casts a number to a boolean object.
 *
 * In other languages:
 * Dutch: [Getal] boolean
 */
ctr_object* ctr_number_to_boolean(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool( myself->value.nvalue );
}

/**
 * String
 *
 * Literal:
 *
 * 'Hello World, this is a String.'
 *
 * A sequence of bytes or characters. In Citrine, strings are UTF-8 aware.
 * You may only use single quotes. To escape a character use the
 * backslash '\' character. Use the special characters ↵ and ⇿ to
 * insert a newline or tab respectively.
 *
 * Strings in Citrine represent a series of bytes. Strings can be
 * interpreted as real bytes or as text depending on the messages
 * send. For instance, the message 'bytes' returns the number of bytes
 * in a string, while the message 'length' returns the number of
 * characters (as defined as separate UTF-8 code points) in a string.
 *
 * In other languages:
 * Dutch: Tekst
 */
ctr_object* ctr_build_string(char* stringValue, long size) {
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
 * [String] bytes
 *
 * Returns the number of bytes in a string, as opposed to
 * length which returns the number of UTF-8 code points (symbols or characters).
 *
 * In other languages:
 * Dutch: [Tekst] bytes
 */
ctr_object* ctr_string_bytes(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float((float)myself->value.svalue->vlen);
}

/**
 * [String] = [other]
 *
 * Returns True if the other string is the same (in bytes).
 *
 * In other languages:
 * Dutch: [Tekst] = [Tekst] | Geeft Waar als beide teksten dezelfde inhoud bevatten.
 */
ctr_object* ctr_string_eq(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* other = ctr_internal_cast2string( argumentList->object );
	if (other->value.svalue->vlen != myself->value.svalue->vlen) {
		return ctr_build_bool(0);
	}
	return ctr_build_bool((strncmp(other->value.svalue->value, myself->value.svalue->value, myself->value.svalue->vlen)==0));
}

/**
 * [String] ≠ [other]
 *
 * Returns True if the other string is not the same (in bytes).
 *
 * In other languages:
 * Dutch: [Tekst] ≠ [Tekst] | Geeft Waar als beide teksten niet dezelfde inhoud bevatten.
 */
ctr_object* ctr_string_neq(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* other = ctr_internal_cast2string( argumentList->object );
	if (other->value.svalue->vlen != myself->value.svalue->vlen) {
		return ctr_build_bool(1);
	}
	return ctr_build_bool(!(strncmp(other->value.svalue->value, myself->value.svalue->value, myself->value.svalue->vlen)==0));
}

/**
 * [String] length
 *
 * Returns the length of the string in symbols.
 * This message is UTF-8 unicode aware. A 4 byte character will be counted as ONE.
 *
 * In other languages:
 * Dutch: [Tekst] lengte | Geeft lengte van tekst in tekens.
 */
ctr_object* ctr_string_length(ctr_object* myself, ctr_argument* argumentList) {
	ctr_size n = ctr_getutf8len(myself->value.svalue->value, (ctr_size) myself->value.svalue->vlen);
	return ctr_build_number_from_float((ctr_number) n);
}

/**
 * [String] + [other]
 *
 * Appends other string to self and returns the resulting
 * string as a new object.
 *
 * In other languages:
 * Dutch: [Tekst] + [Tekst] | Voegt beide teksten samen.
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
 * [String] append: [String].
 *
 * Appends the specified string to itself. This is different from the '+'
 * message, the '+' message adds the specified string while creating a new string.
 * Appends on the other hand modifies the original string.
 *
 * Usage:
 *
 * x := 'Hello '.
 * x append: 'World'.
 * ✎ write: x.
 *
 * In other languages:
 * Dutch: [Tekst] toevoegen: [Tekst] | Voegt tweede tekst toe aan eerste.
 */
ctr_object* ctr_string_append(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* strObject;
	ctr_size n1;
	ctr_size n2;
	char* dest;
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


/**
 * [String] from: [start] length: [length]
 *
 * Returns a portion of a string defined by from
 * and length values.
 * This message is UTF-8 unicode aware.
 *
 * Usage:
 *
 * 'hello' from: 2 length: 3.
 *
 * In other languages:
 * Dutch: [Tekst] van: [Getal] lengte: [Getal] | Antwoord het deel van de tekst tussen de twee posities.
 */
ctr_object* ctr_string_from_length(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* fromPos = ctr_internal_cast2number(argumentList->object);
	ctr_object* length = ctr_internal_cast2number(argumentList->next->object);
	long len = myself->value.svalue->vlen;
	long a = (fromPos->value.nvalue) - 1;
	long b = (length->value.nvalue);
	long ua, ub;
	char* dest;
	ctr_object* newString;
	if (b == 0) return ctr_build_empty_string();
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
	dest = ctr_heap_allocate( ub * sizeof(char) );
	memcpy(dest, (myself->value.svalue->value) + ua, ub);
	newString = ctr_build_string(dest,ub);
	ctr_heap_free( dest );
	return newString;
}

/**
 * [String] offset: [Number]
 *
 * Returns a string without the first X characters.
 *
 * In other languages:
 * Dutch: [Tekst] overslaan: [Getal] | Geeft de tekst vanaf de opgegeven positie.
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
	argument2->object = ctr_build_number_from_float(textLength - (a - 1) );
	result = ctr_string_from_length(myself, argument1);
	ctr_heap_free( argument1 );
	ctr_heap_free( argument2 );
	return result;
}

/**
 * [String] character: [Number]
 *
 * Returns the character at the specified position (UTF8 aware).
 *
 * Usage:
 *
 * ('hello' character: 2).
 *
 * In other languages:
 * Dutch: [Tekst] letter: [Getal] | Geeft de letter op aangegeven positie.
 */
ctr_object* ctr_string_at(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* fromPos = ctr_internal_cast2number(argumentList->object);
	int32_t a = (int32_t) (fromPos->value.nvalue) - 1;
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
 * [String] byte: [Number]
 *
 * Returns the byte at the specified position (in bytes).
 *
 * Usage:
 * ('abc' byte: 1).
 *
 * In other languages:
 * Dutch: [Tekst] byte: [Getal] | Geeft de byte op aangegeven positie.
 */
ctr_object* ctr_string_byte_at(ctr_object* myself, ctr_argument* argumentList) {
	char x;
	ctr_object* fromPos = ctr_internal_cast2number(argumentList->object);
	long a = (fromPos->value.nvalue) - 1;
	long len = myself->value.svalue->vlen;
	if (a >= len) return CtrStdNil;
	if (a < 0) return CtrStdNil;
	x = (char) *(myself->value.svalue->value + a);
	return ctr_build_number_from_float((double)x);
}

/**
 * [String] find: [subject]
 *
 * Returns the index (character number, not the byte!) of the
 * needle in the haystack.
 *
 * Usage:
 *
 * 'find the needle' find: 'needle'.
 *
 * In other languages:
 * Dutch: [Tekst] vind: [Tekst] | Geeft positie van eerste voorkomen deeltekst.
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
	return ctr_build_number_from_float((ctr_number) uchar_index + 1);
}

/**
 * [String] uppercase
 *
 * Returns a new uppercased version of the string.
 * Note that this is just basic ASCII case functionality, this should only
 * be used for internal keys and as a basic utility function. This function
 * DOES NOT WORK WITH UTF8 characters !
 *
 * In other languages:
 * Dutch: [Tekst] hoofdletters | Geeft de tekst in hoofdletters.
 */
ctr_object* ctr_string_to_upper(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* newString = NULL;
	char* str = myself->value.svalue->value;
	size_t  len = myself->value.svalue->vlen;
	char* tstr = ctr_heap_allocate( len * sizeof( char ) );
	int i=0;
	for(i =0; i < len; i++) {
		tstr[i] = toupper(str[i]);
	}
	newString = ctr_build_string(tstr, len);
	ctr_heap_free( tstr );
	return newString;
}


/**
 * [String] lowercase
 *
 * Returns a new lowercased version of the string.
 * Note that this is just basic ASCII case functionality, this should only
 * be used for internal keys and as a basic utility function. This function
 * DOES NOT WORK WITH UTF8 characters !
 *
 * In other languages:
 * Dutch: [Tekst] kleine letters | Geeft de tekst in hoofdletters.
 */
ctr_object* ctr_string_to_lower(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* newString = NULL;
	char* str = myself->value.svalue->value;
	size_t len = myself->value.svalue->vlen;
	char* tstr = ctr_heap_allocate( len * sizeof( char ) );
	int i=0;
	for(i =0; i < len; i++) {
		tstr[i] = tolower(str[i]);
	}
	newString = ctr_build_string(tstr, len);
	ctr_heap_free( tstr );
	return newString;
}

ctr_object* ctr_string_to_string(ctr_object* myself, ctr_argument* argumentList) {
	return myself;
}

/**
 * [String] last: [subject]
 *
 * Returns the index (character number, not the byte!) of the
 * needle in the haystack.
 *
 * Usage:
 *
 * 'find the needle' last: 'needle'.
 *
 * In other languages:
 * Dutch: [Tekst] laatste: [Tekst] | Geeft laatste positie deeltekst.
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
	return ctr_build_number_from_float((float) uchar_index + 1);
}

/**
 * [String] [key]: [value]
 *
 * Replaces the character sequence 'key' with the contents of value.
 * The example will produce the string '$ 10'.
 *
 * Usage:
 *
 * '$ money' money: 10.
 *
 * In other languages:
 * Dutch: [Tekst] [Tekst]: [Tekst] | Vervangt tekst 2 met tekst 3.
 */
ctr_object* ctr_string_fill_in(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* message = ctr_internal_cast2string( argumentList->object );
	ctr_object* slot;

	if ( message->value.svalue->value[message->value.svalue->vlen - 1] == ':' ) {
		slot = ctr_build_string( message->value.svalue->value, message->value.svalue->vlen - 1);
	} else {
		slot = message;
	}
	argumentList->object = slot;
	return ctr_string_replace_with( myself, argumentList );
}

/**
 * [String] replace: [string] with: [other]
 *
 * Replaces needle with replacement in original string.
 * Modifies the original string and returns it as well.
 * To preserve the original string, copy it first.
 *
 * Usage:
 *
 * 'LiLo BootLoader' replace: 'L' with: 'l'.
 *
 * In other languages:
 * Dutch: [Tekst] vervang: [Tekst] door: [Tekst] | Vervangt tekst 2 met 3.
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
		return ctr_build_string(src, hlen);
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
 * [String] pattern: [String] process: [Block] options: [String].
 *
 * Matches the POSIX regular expression in the first argument against
 * the string and executes the specified block on every match passing
 * an array containing the matches.
 *
 * The options parameter can be used to pass specific flags to the
 * regular expression engine. As of the moment of writing this functionality
 * has not been implemented yet. The only flag you can set at this moment is
 * the 'ignore' flag, just a test flag. This flag does not execute the block.
 *
 * Usage:
 *
 * 'hello world' pattern: '([hl])' process: { :arr
 *  	✎ write: (arr join: '|'), end.
 * } options: ''.
 *
 * On every match the block gets executed and the matches are
 * passed to the block as arguments. You can also use this feature to replace
 * parts of the string, simply return the replacement string in your block.
 *
 * In other languages:
 * Dutch: [Tekst] patroon: [Tekst] verwerk: [Codeblok] opties: [Tekst] | Past reguliere expressie toe.
 */
ctr_object* ctr_string_find_pattern_options_do( ctr_object* myself, ctr_argument* argumentList ) {
	regex_t pattern;
	int reti;
	int sticky1, sticky2, sticky3, sticky4;
	int regex_error = 0;
	size_t n = 255;
	size_t i = 0;
	regmatch_t matches[255];
	char* needle = ctr_heap_allocate_cstring( ctr_internal_cast2string( argumentList->object ) );
	char* options = ctr_heap_allocate_cstring( ctr_internal_cast2string( argumentList->next->next->object ) );
	uint8_t olen = strlen( options );
	uint8_t p = 0;
	uint8_t flagIgnore = 0;
	uint8_t flagNewLine = 0;
	uint8_t flagCI = 0;
	for ( p = 0; p < olen; p ++ ) {
		if ( options[p] == '!' ) {
			flagIgnore = 1;
		}
		if ( options[p] == 'n' ) {
			flagNewLine = 1;
		}
		if ( options[p] == 'i' ) {
			flagCI = 1;
		}
	}
	ctr_object* block = argumentList->next->object;
	if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
		ctr_heap_free( needle );
		ctr_heap_free( options );
		CtrStdFlow = ctr_error(CTR_ERR_DIVZERO,0);
		return myself;
	}
	int eflags = REG_EXTENDED;
	if (flagNewLine) eflags |= REG_NEWLINE;
	if (flagCI) eflags |= REG_ICASE;
	reti = regcomp(&pattern, needle, eflags);
	if ( reti ) {
		ctr_heap_free( needle );
		ctr_heap_free( options );
		CtrStdFlow = ctr_error(CTR_ERR_REGEX,0);
		return CtrStdNil;
	}
	char* haystack = ctr_heap_allocate_cstring(myself);
	size_t offset = 0;
	ctr_object* newString = ctr_build_empty_string();
	while( !regex_error && !flagIgnore ) {
		regex_error = regexec(&pattern, haystack + offset , n, matches, 0 );
		if ( regex_error ) break;
		ctr_argument* blockArguments;
		blockArguments = ctr_heap_allocate( sizeof( ctr_argument ) );
		ctr_argument* arrayConstructorArgument;
		arrayConstructorArgument = ctr_heap_allocate( sizeof( ctr_argument ) );
		blockArguments->object = ctr_array_new( CtrStdArray, arrayConstructorArgument );
		for( i = 0; i < n; i ++ ) {
			if ( matches[i].rm_so == -1 ) break;
			ctr_argument* group;
			group = ctr_heap_allocate( sizeof( ctr_argument ) );
			size_t len = (matches[i].rm_eo - matches[i].rm_so);
			char* tmp = ctr_heap_allocate( len + 1 );
			memcpy( tmp, haystack + offset + matches[i].rm_so, len );
			group->object = ctr_build_string_from_cstring( tmp );
			ctr_array_push( blockArguments->object, group );
			ctr_heap_free( group );
			ctr_heap_free( tmp );
		}
		if (matches[0].rm_eo != -1) {
			ctr_argument* arg = ctr_heap_allocate( sizeof( ctr_argument ) );
			arg->object = ctr_build_string( haystack + offset, matches[0].rm_so );
			ctr_string_append( newString, arg );
			offset += matches[0].rm_eo;
			ctr_heap_free( arg );
		}
		sticky1 = block->info.sticky;
		sticky2 = blockArguments->object->info.sticky;
		sticky3 = newString->info.sticky;
		sticky4 = myself->info.sticky;
		block->info.sticky = 1;
		blockArguments->object->info.sticky = 1;
		newString->info.sticky = 1;

		ctr_gc_internal_pin(block);
		ctr_gc_internal_pin(blockArguments->object);
		ctr_gc_internal_pin(newString);
		ctr_gc_internal_pin(myself);
		ctr_object* replacement = replacement = ctr_block_run( block, blockArguments, NULL );
		block->info.sticky = sticky1;
		blockArguments->object->info.sticky = sticky2;
		newString->info.sticky = sticky3;
		myself->info.sticky = sticky4;
		ctr_argument* arg = ctr_heap_allocate( sizeof( ctr_argument ) );
		arg->object = replacement;
		ctr_string_append( newString, arg );
		ctr_heap_free( arg );
		ctr_heap_free(blockArguments);
		ctr_heap_free(arrayConstructorArgument);
	}
	ctr_argument* arg = ctr_heap_allocate( sizeof( ctr_argument ) );
	arg->object = ctr_build_string( haystack + offset, strlen( haystack + offset ) );
	ctr_string_append( newString, arg );
	ctr_heap_free( arg );
	ctr_heap_free( needle );
	ctr_heap_free( haystack );
	ctr_heap_free( options );
	regfree( &pattern );
	return newString;
}

/**
 * [String] pattern: [String] process: [Block].
 *
 * Same as pattern:process:options: but without the options, no flags will
 * be send to the regex engine.
 *
 * In other languages:
 * Dutch: [Tekst] patroon: [Tekst] verwerk: [Codeblok] | Past reguliere expressie toe.
 */
ctr_object* ctr_string_find_pattern_do( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_argument* no_options = ctr_heap_allocate( sizeof( ctr_argument ) );
	argumentList->next->next->object = ctr_build_empty_string();
	ctr_object* answer;
	answer = ctr_string_find_pattern_options_do( myself, argumentList );
	ctr_heap_free( no_options );
	return answer;
}

/**
 * [String] contains: [String]
 *
 * Returns True if the other string is a substring.
 *
 * In other languages:
 * Dutch: [Tekst] bevat: [Tekst]
 * geeft Waar terug als het ontvangende tekstobject
 * de aangegeven tekst bevat.
 */
ctr_object* ctr_string_contains( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_build_bool(
		ctr_internal_cast2number(
			ctr_string_index_of( myself, argumentList )
		)->value.nvalue > 0
	);
}

/**
 * [String] matches: [String].
 *
 * Tests the pattern against the string and returns True if there is a match
 * and False otherwise.
 * In the example: match will be True because there is a space in 'Hello World'.
 *
 * Usage:
 *
 * ☞ match := 'Hello World' matches: '[:space:]'.
 *
 * In other languages:
 * Dutch: [Tekst] patroon: [Tekst]
 * Geeft Waar terug als het ontvangende tekstobject overeenkomt
 * met de reguliere uitdrukking.
 */
ctr_object* ctr_string_contains_pattern( ctr_object* myself, ctr_argument* argumentList ) {
	regex_t pattern;
	int regex_error = 0;
	int result = 0;
	char* error_message = ctr_heap_allocate( 255 );
	char* needle = ctr_heap_allocate_cstring( ctr_internal_cast2string( argumentList->object ) );
	char* haystack = ctr_heap_allocate_cstring(myself);
	ctr_object* answer;
	regex_error = regcomp(&pattern, needle, REG_EXTENDED);
	if ( regex_error ) {
		CtrStdFlow = ctr_error( CTR_ERR_REGEX, 0 );
		answer = CtrStdNil;
	} else {
		result = regexec(&pattern, haystack, 0, NULL, 0 );
		if ( !result ) {
			answer = ctr_build_bool( 1 );
		} else if ( result == REG_NOMATCH ) {
			answer = ctr_build_bool( 0 );
		} else {
			CtrStdFlow = ctr_error( error_message, 0 );
			answer = CtrStdNil;
		}
	}
	regfree( &pattern );
	ctr_heap_free( error_message );
	ctr_heap_free( needle );
	ctr_heap_free( haystack );
	return answer;
}

/**
 * [String] remove surrounding spaces
 *
 * Trims a string. Removes surrounding white space characters
 * from string and returns the result as a new string object.
 * The example above will strip all white space characters from the
 * recipient on both sides of the text.
 *
 * Usage:
 *
 * ' hello ' remove surrounding spaces.
 *
 * In other languages:
 * Dutch: [Tekst] verwijder omliggende spaties | verwijderd witruimte links en rechts.
 */
ctr_object* ctr_string_trim(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* newString = NULL;
	char* str = myself->value.svalue->value;
	long  len = myself->value.svalue->vlen;
	long i, begin, end, tlen;
	char* tstr;
	if (len == 0) return ctr_build_empty_string();
	i = 0;
	while(i < len && isspace(*(str+i))) i++;
	begin = i;
	i = len - 1;
	while(i > begin && isspace(*(str+i))) i--;
	end = i + 1;
	tlen = (end - begin);
	tstr = ctr_heap_allocate( tlen * sizeof( char ) );
	memcpy(tstr, str+begin, tlen);
	newString = ctr_build_string(tstr, tlen);
	ctr_heap_free( tstr );
	return newString;
}

/**
 * [String] number
 *
 * Converts string to a number.
 *
 * In other languages:
 * Dutch: [Tekst] getal | Geeft de getalwaarde van de tekst of anders 0.
 */
ctr_object* ctr_string_to_number(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_string(myself->value.svalue->value, myself->value.svalue->vlen, 1);
}

ctr_object* ctr_string_in_to_number(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_string(myself->value.svalue->value, myself->value.svalue->vlen, 0);
}

/**
 * [String] boolean
 *
 * Converts string to boolean
 *
 * In other languages:
 * Dutch: [Tekst] boolean | Geeft de booleaanse waarheid van de tekst.
 */
ctr_object* ctr_string_to_boolean(ctr_object* myself, ctr_argument* argumentList) {
	if ( myself->value.svalue->vlen == 0 ) return ctr_build_bool(0);
	return ctr_build_bool( 1 );
}

/**
 * [String] split: [String]
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
	ctr_argument* arg;
	char* elem;
	ctr_object* arr = ctr_array_new(CtrStdArray, NULL);
	long i;
	long j = 0;
	char* buffer = ctr_heap_allocate( sizeof(char)*len );
	for(i=0; i<len; i++) {
		buffer[j] = str[i];
		j++;
		if (ctr_internal_memmem(buffer, j, dstr, dlen, 0)!=NULL) {
			elem = ctr_heap_allocate( sizeof( char ) * ( j - dlen ) );
			memcpy(elem,buffer,j-dlen);
			arg = ctr_heap_allocate( sizeof( ctr_argument ) );
			arg->object = ctr_build_string(elem, j-dlen);
			ctr_array_push(arr, arg);
			ctr_heap_free( arg );
			ctr_heap_free( elem );
			j=0;
		}
	}
	if (j>0) {
		elem = ctr_heap_allocate( sizeof( char ) * j );
		memcpy(elem,buffer,j);
		arg = ctr_heap_allocate( sizeof( ctr_argument ) );
		arg->object = ctr_build_string(elem, j);
		ctr_array_push(arr, arg);
		ctr_heap_free( arg );
		ctr_heap_free( elem );
	}
	ctr_heap_free( buffer );
	return arr;
}

/**
 * [String] characters.
 *
 * Splits the string in UTF-8 characters and returns
 * those as an array.
 *
 * Usage:
 *
 * a := 'abc' characters.
 * a count.
 *
 * In other languages:
 * Dutch: [Tekst] letters | Geeft het aantal letters dat de tekst bevat.
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
 * [String] list
 *
 * Returns an array of bytes representing the string.
 *
 * In other languages:
 * Dutch: [Tekst] bytereeks | Geeft de bytes als reeks waaruit de tekst bestaat.
 */
ctr_object* ctr_string_to_byte_array( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_size i;
	ctr_object* arr;
	ctr_argument* newArgumentList;
	arr = ctr_array_new(CtrStdArray, NULL);
	newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	i = 0;
	while( i < myself->value.svalue->vlen ) {
		newArgumentList->object = ctr_build_number_from_float( (double) (uint8_t) *(myself->value.svalue->value + i) );
		ctr_array_push( arr, newArgumentList );
		i ++;
	}
	ctr_heap_free( newArgumentList );
	return arr;
}

/**
 * [String] append byte: [Number].
 *
 * Appends a raw byte to a string.
 *
 * In other languages:
 * Dutch: [Tekst] voeg byte toe: [Getal] | Voegt bytewaarde aangegeven door getal toe aan tekst.
 */
ctr_object* ctr_string_append_byte( ctr_object* myself, ctr_argument* argumentList ) {
	char* dest;
	char byte;
	byte = (uint8_t) ctr_internal_cast2number( argumentList->object )->value.nvalue;
	dest = ctr_heap_allocate( myself->value.svalue->vlen + 1 );
	memcpy( dest, myself->value.svalue->value, myself->value.svalue->vlen );
	*( dest + myself->value.svalue->vlen ) = byte;
	if ( myself->value.svalue->vlen > 0 ) {
		ctr_heap_free( myself->value.svalue->value );
	}
	myself->value.svalue->value = dest;
	myself->value.svalue->vlen++;
	return myself;
}

/**
 * [String] compare: [String]
 *
 * Compares a string using the UTF-8 compatible strcmp function.
 * Returns less than zero if receiving object comes before specified string,
 * higher than zero if receiving string comes after and 0 if equal.
 *
 * Usage:
 *
 * word compare: other.
 *
 * In other languages:
 * Dutch: [Tekst] vergelijk: [Tekst] | Geeft <0 terug als ontvanger voor tekst komt, >0 erna, =0 gelijk.
 */
ctr_object* ctr_string_compare( ctr_object* myself, ctr_argument* argumentList ) {
	argumentList->object = ctr_internal_cast2string(argumentList->object);
	ctr_size maxlen;
	if (myself->value.svalue->vlen < argumentList->object->value.svalue->vlen) {
		maxlen = myself->value.svalue->vlen;
	} else {
		maxlen = argumentList->object->value.svalue->vlen;
	}
	return ctr_build_number_from_float( (ctr_number) strncmp(
		myself->value.svalue->value,
		ctr_internal_cast2string(argumentList->object)->value.svalue->value,
		maxlen
	) );
}

/**
 * [String] < [String]
 *
 * Returns True if the first String comes before the latter
 * alphabetically. The actual comparison is based on the UTF-8 compatible
 * function strcmp.
 *
 * In other languages:
 * Dutch: [Tekst] < [Tekst] | Geeft Waar als eerste voor tweede komt alfabetisch.
 */
ctr_object* ctr_string_before(ctr_object* myself, ctr_argument* argumentList ) {
	if ( ctr_string_compare( myself, argumentList )->value.nvalue < 0 ) {
		return ctr_build_bool( 1 );
	}
	return ctr_build_bool( 0 );
}

/**
 * [String] ≤ [String]
 *
 * Returns True if the first String comes before or at the same
 * position as the latter alphabetically. The actual comparison is based on the UTF-8 compatible
 * function strcmp.
 *
 * In other languages:
 * Dutch: [Tekst] ≤ [Tekst] | Geeft Waar als eerste voor tweede komt alfabetisch of gelijk.
 */
ctr_object* ctr_string_before_or_same(ctr_object* myself, ctr_argument* argumentList ) {
	if ( ctr_string_compare( myself, argumentList )->value.nvalue <= 0 ) {
		return ctr_build_bool( 1 );
	}
	return ctr_build_bool( 0 );
}

/**
 * [String] > [String]
 *
 * Returns True if the first String comes after the latter
 * alphabetically. The actual comparison is based on the UTF-8 compatible
 * function strcmp.
 *
 * In other languages:
 * Dutch: [Tekst] > [Tekst] | Geeft Waar als eerste na tweede komt alfabetisch.
 */
ctr_object* ctr_string_after(ctr_object* myself, ctr_argument* argumentList ) {
	if ( ctr_string_compare( myself, argumentList )->value.nvalue > 0 ) {
		return ctr_build_bool( 1 );
	}
	return ctr_build_bool( 0 );
}

/**
 * [String] ≥ [String]
 *
 * Returns True if the first String comes after or at the same position as the latter
 * alphabetically. The actual comparison is based on the UTF-8 compatible
 * function strcmp.
 *
 * In other languages:
 * Dutch: [Tekst] ≥ [Tekst] | Geeft Waar als eerste gelijk of na tweede komt alfabetisch.
 */
ctr_object* ctr_string_after_or_same(ctr_object* myself, ctr_argument* argumentList ) {
	if ( ctr_string_compare( myself, argumentList )->value.nvalue >= 0 ) {
		return ctr_build_bool( 1 );
	}
	return ctr_build_bool( 0 );
}

 /**
 * @internal
 *
 * Escapes all single quotes in a string. Sending this message to a
 * string will cause all single quotes (') to be replaced with (\').
 */
ctr_object* ctr_string_quotes_escape(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* answer;
	char* str;
	ctr_size len;
	ctr_size i;
	ctr_size j;
	len = myself->value.svalue->vlen;
	for( i = 0; i < myself->value.svalue->vlen; i++ ) {
		if ( *(myself->value.svalue->value + i) == '\'' ) {
				len++;
		}
	}
	str = ctr_heap_allocate( len + 1 );
	j = 0;
	for( i = 0; i < myself->value.svalue->vlen; i++ ) {
		if ( *(myself->value.svalue->value + i) == '\'' ) {
				str[j+i] = '\\';
				j++;
		}
		str[j+i] = *(myself->value.svalue->value + i);
	}
	answer = ctr_build_string_from_cstring( str );
	ctr_heap_free( str );
	return answer;
}


/**
 * [String] hash: [String]
 *
 * Returns the hash of the recipient String using the specified key.
 * The default hash in Citrine is the SipHash which is also used internally.
 * SipHash can protect against hash flooding attacks.
 *
 * In other languages:
 * Dutch: [Tekst] kluts [Tekst] | Geeft de kluts (SipHash) terug voor de gespecificeerde sleutel
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
 * Block
 *
 * Literal:
 *
 * { parameters (if any) here... code here... }
 *
 * each parameter has to be prefixed with
 * a colon (:).
 *
 * Usage:
 *
 * { ✎ write: 'a simple code block'. } run.
 * { :param ✎ write: param. } apply: 'write this!'.
 * { :a :b ↲ a + b. } apply: 1 and: 2.
 * { :a :b :c ↲ a + b + c. } apply: 1 and: 2 and: 3.
 *
 * In other languages:
 * Dutch: een letterlijk codeblok begint met { en eindigt met }.
 * De parameters komen na de openingsaccolade en worden voorafgegaan door
 * een dubbele punt. Om vanuit een codeblok terug te keren naar het
 * bovenliggende programma en een antwoord terug te sturen gebruikt men
 * het terugkeersymbool: ↲.
 */
ctr_object* ctr_build_block(ctr_tnode* node) {
	ctr_object* codeBlockObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTBLOCK);
	codeBlockObject->value.block = node;
	codeBlockObject->link = CtrStdBlock;
	return codeBlockObject;
}

/**
 * [Block] apply: [object]
 *
 * Runs a block of code using the specified object as a parameter.
 * If you run a block using the messages 'run' or 'apply:', me/my will
 * refer to the block itself instead of the containing object.
 *
 * In other languages:
 * Dutch: [Codeblok] toepassen: [Object] | Start het codeblok met het opgegeven object als argument.
 */
 
int xx = 0;
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
		while(1) {
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
	ctr_assign_value_to_local(ctr_build_string_from_cstring( CTR_DICT_THIS_BLOCK ), myself ); /* otherwise running block may get gc'ed. */
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
 * [Block] while: [block]
 *
 * Runs a block of code, depending on the outcome runs the other block
 * as long as the result of the first one equals boolean True.
 * Example: Here we increment variable x by one until it reaches 6.
 * While the number x is lower than 6 we keep incrementing it.
 * Don't forget to use the return ↲ symbol in the first block.
 *
 * Usage:
 *
 * ☞ x := 0.
 * { x add: 1. } while: { ↲ (x < 6). }.
 *
 * In other languages:
 * Dutch: [Codeblok] zolang: [Codeblok]
 * Draait het codeblok net zolang totdat de uitkomst van het opgegeven blok negatief wordt.
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

/**
 * [Block] run
 *
 * Sending the unary message 'run' to a block will cause it to execute.
 * The run message takes no arguments, if you want to use the block as a function
 * and send arguments, consider using the applyTo-family of messages instead.
 * This message just simply runs the block of code without any arguments.
 * In the example we will run the code inside the block and display
 * the greeting.
 *
 * Usage:
 *
 * { ✎ write: 'Hello World'. } run.
 *
 * In other languages:
 * Dutch: [Codeblok] start. | Start het blok code.
 */
ctr_object* ctr_block_runIt(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* result;
	result = ctr_block_run(myself, argumentList, myself); /* here me/my refers to block itself not object - this allows closures. */
	if (CtrStdFlow == CtrStdBreak || CtrStdFlow == CtrStdContinue) CtrStdFlow = NULL; /* consume break */
	return result;
}

/**
 * [Block] set: [name] value: [object]
 *
 * Sets a variable in a block of code. This how you can get closure-like
 * functionality.
 * In the example we assign a block to a variable named 'shout'.
 * We assign the string 'hello' to the variable 'message' inside the block.
 * When we invoke the block 'shout' by sending the run message without any
 * arguments it will display the string: 'hello!!!'.
 * Similarly, you could use this technique to create a block that returns a
 * block that applies a formula (for instance simple multiplication) and then set the
 * multiplier to use in the formula. This way, you could create a block
 * building 'formula blocks'. This is how you implement use closures
 * in Citrine.
 *
 * Usage:
 *
 * shout := { ✎ write: (my message + '!!!'). }.
 * shout set: 'message' value: 'hello'.
 * shout run.
 *
 * In other languages:
 * Dutch: [Codeblok] gebruik: [Tekst] waarde: [Object]
 * Stelt een variabele in die binnen het blok bruikbaar wordt.
 */
ctr_object* ctr_block_set(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* key = ctr_internal_cast2string(argumentList->object);
	ctr_object* value = argumentList->next->object;
	ctr_internal_object_set_property(myself, key, value, 0);
	return myself;
}

/**
 * [Block] error: [object].
 *
 * Sets error flag on a block of code.
 * This will throw an error / exception.
 * You can attach an object to the error, for instance
 * an error message.
 *
 * Usage:
 *
 * {
 *   this code block error: 'oops!'.
 * } catch: { :errorMessage
 *   ✎ write: errorMessage.
 * }, run.
 *
 * In other languages:
 * Dutch: [Codeblok] fout: [Object] | Laat een kunstmatige een fout of uitzondering optreden.
 * Om te verwijzen naar het huidige blok code gebruik: dit codeblok.
 * Voorbeeld:
 *
 * {
 *   dit codeblock fout: 'oeps!'.
 * } afhandelen: { :fout
 *   ✎ schrijf: fout.
 * }, start.
 */
ctr_object* ctr_block_error(ctr_object* myself, ctr_argument* argumentList) {
	CtrStdFlow = argumentList->object;
	CtrStdFlow->info.sticky = 1;
	return myself;
}

/**
 * [Block] catch: [otherBlock]
 *
 * Associates an error clause to a block.
 * If an error (exception) occurs within the block this block will be
 * executed.
 *
 * Usage:
 *
 * {
 *    ☞ z := 4 / 0.
 * } catch: { :e
 *    ✎ write: e, end.
 * }, run.
 *
 * In other languages:
 * Dutch: [Codeblok] afhandelen: [Codeblok]
 * Stelt een codeblok in dat gebruikt moet worden door het ontvangende blok
 * indien er een fout opgetreden is.
 */
ctr_object* ctr_block_catch(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* catchBlock = argumentList->object;
	ctr_internal_object_delete_property(myself, ctr_build_string_from_cstring( "catch" ), 0 );
	ctr_internal_object_add_property(myself, ctr_build_string_from_cstring( "catch" ), catchBlock, 0 );
	return myself;
}

/**
 * [Block] string
 *
 * Returns a string representation of the Block. This basic behavior, part
 * of any object will just return [Block]. Other objects typically override this
 * behavior with more useful implementations.
 *
 * In other languages:
 * Dutch: [Codeblok] tekst | Geeft tekstuele weergave van een codeblok.
 */
ctr_object* ctr_block_to_string(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_string_from_cstring( CTR_SYM_BLOCK );
}
