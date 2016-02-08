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
 * Nil
 *
 * Literal: 
 * 
 * Nil
 */
ctr_object* ctr_build_nil() {
	return CtrStdNil;
}

/**
 * [Nil] isNil
 *
 * Nil always answers this message with a boolean object 'True'.
 */
ctr_object* ctr_nil_is_nil(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(1);
}

/**
 * Object
 * 
 * This is the base object, the parent of all other objects.
 * It contains essential object oriented programming features.
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
 */
ctr_object* ctr_object_type(ctr_object* myself, ctr_argument* argumentList) {

	 switch(myself->info.type){
	       case CTR_OBJECT_TYPE_OTNIL:
		        return ctr_build_string_from_cstring("Nil");
	       case CTR_OBJECT_TYPE_OTBOOL:
		        return ctr_build_string_from_cstring("Boolean");
	       case CTR_OBJECT_TYPE_OTNUMBER:
		        return ctr_build_string_from_cstring("Number");
	       case CTR_OBJECT_TYPE_OTSTRING:
		        return ctr_build_string_from_cstring("String");
	       case CTR_OBJECT_TYPE_OTBLOCK:
	       case CTR_OBJECT_TYPE_OTNATFUNC:
		        return ctr_build_string_from_cstring("Block");
           default:
	            return ctr_build_string_from_cstring("Object");
    }
}

/**
 * [Object] equals: [other]
 *
 * Tests whether the current instance is the same as
 * the argument.
 * 
 * Alias: =
 * 
 * Usage:
 * object equals: other
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
 */
ctr_object* ctr_object_myself(ctr_object* myself, ctr_argument* argumentList) {
	return myself;
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
 * object on: 'greet' do: {\ ... }.
 * object on: 'between:and:' do: {\ ... }.
 * 
 */
ctr_object* ctr_object_on_do(ctr_object* myself, ctr_argument* argumentList) {
	ctr_argument* nextArgument;
	ctr_object* methodBlock;
	ctr_object* methodName = argumentList->object;
	if (methodName->info.type != CTR_OBJECT_TYPE_OTSTRING) {
		CtrStdError = ctr_build_string_from_cstring("Expected on: argument to be of type string.");
		return myself;
	}
	nextArgument = argumentList->next;
	methodBlock = nextArgument->object;
	if (methodBlock->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
		CtrStdError = ctr_build_string_from_cstring("Expected argument do: to be of type block.");
		return myself;
	}
	ctr_internal_object_add_property(myself, methodName, methodBlock, 1);
	return myself;
}

/**
 * [Object] respondTo: [String]
 * 
 * Variations:
 * 
 * [Object] respondTo: [String] and: [String]
 * [Object] respondTo: [String] and: [String] and: [String]
 * 
 * Default respond-to implemention, does nothing.
 */
ctr_object* ctr_object_respond(ctr_object* myself, ctr_argument* argumentList) {
	return myself;
}

/**
 * [Object] isNil
 *
 * Default isNil implementation.
 *
 * Always returns boolean object False.
 */
ctr_object* ctr_object_is_nil(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(0);
}

/**
 * Boolean
 * 
 * Literal: 
 * 
 * True 
 * False
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
 * (True = False) ifFalse: {\ Pen write: 'This is not True!'. }.
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
 * (True != False) ifTrue: {\ Pen write: 'This is not True!'. }.
 */
ctr_object* ctr_bool_neq(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(ctr_internal_cast2bool(argumentList->object)->value.bvalue != myself->value.bvalue);
}

/**
 * [Boolean] toString
 *
 * Simple cast function.
 */
ctr_object* ctr_bool_to_string(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_cast2string(myself);
}

/**
 * [Boolean] break
 * 
 * Breaks out of the current block and bubbles up to the parent block if
 * the value of the receiver equals boolean True.
 * 
 * Usage:
 * 
 * (iteration > 10) break. #breaks out of loop after 10 iterations
 */
ctr_object* ctr_bool_break(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.bvalue) {
		CtrStdError = CtrStdBreak; /* If error = Break it's a break, there is no real error. */
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
 */
ctr_object* ctr_bool_continue(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.bvalue) {
		CtrStdError = CtrStdContinue; /* If error = Continue, then it breaks only one iteration (return). */
	}
	return myself;
}

/**
 * [Boolean] ifTrue: [block]
 *
 * Executes a block of code if the value of the boolean
 * object is True.
 *
 * Usage:
 * (some expression) ifTrue: {\ ... }.
 *
 */
ctr_object* ctr_bool_iftrue(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.bvalue) {
		ctr_object* codeBlock = argumentList->object;
		ctr_argument* arguments = CTR_CREATE_ARGUMENT();
		arguments->object = myself;
		return ctr_block_run(codeBlock, arguments, codeBlock);
	}
	if (CtrStdError == CtrStdBreak) CtrStdError = NULL; /* consume break */
	return myself;
}

/**
 * [Boolean] ifFalse: [block]
 *
 * Executes a block of code if the value of the boolean
 * object is True.
 *
 * Usage:
 * (some expression) ifFalse: {\ ... }.
 *
 */
ctr_object* ctr_bool_ifFalse(ctr_object* myself, ctr_argument* argumentList) {
	if (!myself->value.bvalue) {
		ctr_object* codeBlock = argumentList->object;
		ctr_argument* arguments = CTR_CREATE_ARGUMENT();
		arguments->object = myself;
		return ctr_block_run(codeBlock, arguments, codeBlock);
	}
	if (CtrStdError == CtrStdBreak) CtrStdError = NULL; /* consume break */
	return myself;
}

/**
 * [Boolean] not
 *
 * Returns the opposite of the current value.
 *
 * Usage:
 * True := False not.
 *
 */
ctr_object* ctr_bool_not(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(!myself->value.bvalue);
}

/**
 * [Boolean] flip
 *
 * 'Flips a coin'. Returns a random boolean value True or False.
 *
 * Usage:
 * coinLandsOn := (Boolean flip).
 */
ctr_object* ctr_bool_flip(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool((rand() % 2));
}

/**
 * [Boolean] either: [this] or: [that]
 *
 * Returns argument #1 if boolean value is True and argument #2 otherwise.
 *
 * Usage:
 * Pen write: 'the coin lands on: ' + (Boolean flip either: 'head' or: 'tail').
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
 * a && b
 *
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
 * a || b
 */
ctr_object* ctr_bool_or(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* other = ctr_internal_cast2bool(argumentList->object);	
	return ctr_build_bool((myself->value.bvalue || other->value.bvalue));
}

/**
 * [Boolean] xor: [other]
 *
 * Returns True if either the object value is True or the
 * argument is True but not both.
 *
 * Usage:
 *
 * a xor: b
 */
ctr_object* ctr_bool_xor(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* other = ctr_internal_cast2bool(argumentList->object);	
	return ctr_build_bool((myself->value.bvalue ^ other->value.bvalue));
}

/**
 * [Boolean] toNumber
 *
 * Returns 0 if boolean is False and 1 otherwise.
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
ctr_object* ctr_build_number_from_string(char* str, ctr_size length) {
	char* numCStr;
	ctr_object* numberObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNUMBER);
	/* turn string into a C-string before feeding it to atof */
	numCStr = (char*) calloc(40, sizeof(char));
	memcpy(numCStr, str, length);
	numberObject->value.nvalue = atof(numCStr);
	numberObject->link = CtrStdNumber;
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
 * Returns True if the number is higher than other number.
 */
ctr_object* ctr_number_higherThan(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue > otherNum->value.nvalue));
}

/**
 * [Number] >= [other]
 * 
 * Returns True if the number is higher than or equal to other number.
 */
ctr_object* ctr_number_higherEqThan(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue >= otherNum->value.nvalue));
}

/**
 * [Number] < [other]
 * 
 * Returns True if the number is less than other number.
 */
ctr_object* ctr_number_lowerThan(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue < otherNum->value.nvalue));
}

/**
 * [Number] <= [other]
 * 
 * Returns True if the number is less than or equal to other number.
 */
ctr_object* ctr_number_lowerEqThan(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue <= otherNum->value.nvalue));
}

/**
 * [Number] = [other]
 * 
 * Returns True if the number equals the other number.
 */
ctr_object* ctr_number_eq(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool(myself->value.nvalue == otherNum->value.nvalue);
}

/**
 * [Number] != [other]
 * 
 * Returns True if the number does not equal the other number.
 */
ctr_object* ctr_number_neq(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool(myself->value.nvalue != otherNum->value.nvalue);
}

/**
 * [Number] between: [low] and: [high]
 *
 * Returns True if the number instance has a value between the two
 * specified values.
 *
 * Usage:
 *
 * q between: x and: y
 */
ctr_object* ctr_number_between(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_argument* nextArgumentItem = argumentList->next;
	ctr_object* nextArgument = ctr_internal_cast2number(nextArgumentItem->object);
	return ctr_build_bool((myself->value.nvalue >=  otherNum->value.nvalue) && (myself->value.nvalue <= nextArgument->value.nvalue));
}

/**
 * [Number] odd
 * 
 * Returns True if the number is odd and False otherwise.
 */
ctr_object* ctr_number_odd(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool((int)myself->value.nvalue % 2);
}

/**
 * [Number] even
 * 
 * Returns True if the number is even and False otherwise.
 */
ctr_object* ctr_number_even(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(!((int)myself->value.nvalue % 2));
}

/**
 * [Number] + [Number]
 * 
 * Adds the other number to the current one. Returns a new
 * number object.
 */
ctr_object* ctr_number_add(ctr_object* myself, ctr_argument* argumentList) {
	ctr_argument* newArg;
	ctr_object* otherNum = argumentList->object;
	ctr_number a;
	ctr_number b;
	ctr_object* strObject;
	if (otherNum->info.type == CTR_OBJECT_TYPE_OTSTRING) {
		strObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTSTRING);
		strObject = ctr_internal_cast2string(myself);
		newArg = CTR_CREATE_ARGUMENT();
		newArg->object = otherNum;
		return ctr_string_concat(strObject, newArg);
	}
	a = myself->value.nvalue;
	b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a+b));
}

/**
 * [Number] inc: [Number]
 * 
 * Increases the number ITSELF by the specified amount, this message will change the
 * value of the number object itself instead of returning a new number.
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
 */
ctr_object* ctr_number_minus(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_number a = myself->value.nvalue;
	ctr_number b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a-b));
}

/**
 * [Number] dec: [number]
 * 
 * Decreases the number ITSELF by the specified amount, this message will change the
 * value of the number object itself instead of returning a new number.
 */
ctr_object* ctr_number_dec(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	myself->value.nvalue -= otherNum->value.nvalue;
	return myself;
}

/**
 * [Number] * [Number or Block]
 *
 * Multiplies the number by the specified divider. Returns a new
 * number object.
 *
 * If the argument is a block, this message is an alias for times:
 */
ctr_object* ctr_number_multiply(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum;
	ctr_number a;
	ctr_number b;
	CTR_MIRROR_CALL(CTR_OBJECT_TYPE_OTBLOCK, ctr_block_times, mirror1);
	otherNum = ctr_internal_cast2number(argumentList->object);
	a = myself->value.nvalue;
	b = otherNum->value.nvalue;
	return ctr_build_number_from_float(a*b);
}

/**
 * [Number] times: [Block]
 *
 * Runs the block N times, where N is the value of the Number.
 */
ctr_object* ctr_number_times(ctr_object* myself, ctr_argument* argumentList) {
	CTR_MIRROR_CALL(CTR_OBJECT_TYPE_OTBLOCK, ctr_block_times, mirror1);
	return myself;
}

/**
 * [Number] mul: [Number]
 * 
 * Multiplies the number ITSELF by multiplier, this message will change the
 * value of the number object itself instead of returning a new number.
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
 */
ctr_object* ctr_number_divide(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_number a = myself->value.nvalue;
	ctr_number b = otherNum->value.nvalue;
	if (b == 0) {
		CtrStdError = ctr_build_string_from_cstring("Division by zero.");
		return myself;
	}
	return ctr_build_number_from_float((a/b));
}

/**
 * [Number] div: [Number]
 * 
 * Divides the number ITSELF by divider, this message will change the
 * value of the number object itself instead of returning a new number.
 */
ctr_object* ctr_number_div(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	if (otherNum->value.nvalue == 0) {
		CtrStdError = ctr_build_string_from_cstring("Division by zero.");
		return myself;
	}
	myself->value.nvalue /= otherNum->value.nvalue;
	return myself;
}

/**
 * [Number] modulo: [modulo]
 *
 * Returns the modulo of the number.
 */
ctr_object* ctr_number_modulo(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_number a = myself->value.nvalue;
	ctr_number b = otherNum->value.nvalue;
	if (b == 0) {
		CtrStdError = ctr_build_string_from_cstring("Division by zero.");
		return myself;
	}
	return ctr_build_number_from_float(fmod(a,b));
}

/**
 * [Number] pow: [power]
 *
 * Returns the number to the specified power.
 */
ctr_object* ctr_number_pow(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_number a = myself->value.nvalue;
	ctr_number b = otherNum->value.nvalue;
	return ctr_build_number_from_float(pow(a,b));
}

/**
 * [Number] max: [other]
 *
 * Returns the biggest number of the two.
 *
 * Usage:
 *
 * x := 6 max: 4. #x is 6
 * x := 6 max: 7. #x is 7
 */
ctr_object* ctr_number_max(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_number a = myself->value.nvalue;
	ctr_number b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a >= b) ? a : b);
}

/**
 * [Number] min: [other]
 *
 * Returns a the smallest number.
 *
 * Usage:
 *
 * x := 6 min: 4. #x is 4
 * x := 6 min: 7. #x is 7
 */
ctr_object* ctr_number_min(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_number a = myself->value.nvalue;
	ctr_number b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a <= b) ? a : b);
}

/**
 * [Number] factorial
 *
 * Calculates the factorial of a number.
 */
ctr_object* ctr_number_factorial(ctr_object* myself, ctr_argument* argumentList) {
	ctr_number t = myself->value.nvalue;
	int i;
	ctr_number a = 1;
	for(i = (int) t; i > 0; i--) {
		a = a * i;
	}
	return ctr_build_number_from_float(a);
}

/**
 * [Number] to: [number] by: [step] do: [block]
 *
 * Runs the specified block for each step it takes to go from
 * the start value to the target value using the specified step size.
 * This is basically how you write for-loops in Citrine.
 * 
 * Usage:
 *
 * 1 to: 5 by: 1 do: { step | Pen write: 'this is step #'+step. }.
 */
ctr_object* ctr_number_to_by_do(ctr_object* myself, ctr_argument* argumentList) {
	double startValue = myself->value.nvalue;
	double endValue   = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	double incValue   = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
	double curValue   = startValue;
	ctr_object* codeBlock = argumentList->next->next->object;
	ctr_argument* arguments;
	int forward = 0;
	if (startValue == endValue) return myself;
	forward = (startValue < endValue);
	if (codeBlock->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
		CtrStdError = ctr_build_string_from_cstring("Expected block.\0");
		return myself;
	}
	while(((forward && curValue <= endValue) || (!forward && curValue >= endValue)) && !CtrStdError) {
		arguments = CTR_CREATE_ARGUMENT();
		arguments->object = ctr_build_number_from_float(curValue);
		ctr_block_run(codeBlock, arguments, codeBlock);
		if (CtrStdError == CtrStdContinue) CtrStdError = NULL; /* consume continue and go on */
		curValue += incValue;
	}
	if (CtrStdError == CtrStdBreak) CtrStdError = NULL; /* consume break */
	return myself;
}

/**
 * [Number] floor
 *
 * Returns the next highest integer number by rounding down.
 */
ctr_object* ctr_number_floor(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(floor(myself->value.nvalue));
}

/**
 * [Number] ceil
 *
 * Returns the next highest integer number by rounding up.
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
 * [Number] abs
 * 
 * Returns the absolute value of the number.
 */
ctr_object* ctr_number_abs(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(fabs(myself->value.nvalue));
}

/**
 * [Number] sqrt
 * 
 * Returns the square root of the number.
 */
ctr_object* ctr_number_sqrt(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(sqrt(myself->value.nvalue));
}

/**
 * [Number] exp
 * 
 * Returns the exponent of the number.
 */
ctr_object* ctr_number_exp(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(exp(myself->value.nvalue));
}

/**
 * [Number] sin
 * 
 * Returns the sine of the number.
 */

ctr_object* ctr_number_sin(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(sin(myself->value.nvalue));
}

/**
 * [Number] cos
 * 
 * Returns the cosine of the number.
 */
ctr_object* ctr_number_cos(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(cos(myself->value.nvalue));
}

/**
 * [Number] tan
 *
 * Caculates the tangent of a number.
 */
ctr_object* ctr_number_tan(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(tan(myself->value.nvalue));
}

/**
 * [Number] atan
 * 
 * Caculates the atan of a number.
 */
ctr_object* ctr_number_atan(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(atan(myself->value.nvalue));
}

/**
 * [Number] log
 *
 * Calculates the logarithm of a number.
 */
ctr_object* ctr_number_log(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(log(myself->value.nvalue));
}

/**
 * [Number] toString
 *
 * Wrapper for cast function.
 */
ctr_object* ctr_number_to_string(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_cast2string(myself);
}

/**
 * [Number] toBoolean
 *
 * Casts a number to a boolean object.
 */
ctr_object* ctr_number_to_boolean(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_cast2bool(myself);
}

/**
 * String
 *
 * Literal:
 * 
 * 'Hello World, this is a String.'
 * 
 * A sequence of characters. In Citrine, strings are UTF-8 aware.
 * You may only use single quotes. To escape a character use the
 * backslash '\' character.
 * 
 */
ctr_object* ctr_build_string(char* stringValue, long size) {
	ctr_object* stringObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTSTRING);
	if (size != 0) {
		stringObject->value.svalue->value = malloc(size*sizeof(char));
		memcpy(stringObject->value.svalue->value, stringValue, size);
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
 * [String] bytes
 *
 * Returns the number of bytes in a string, as opposed to
 * length which returns the number of UTF-8 code points (symbols or characters).
 */
ctr_object* ctr_string_bytes(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float((float)myself->value.svalue->vlen);
}

/**
 * [String] = [other]
 *
 * Returns True if the other string is the same (in bytes).
 */
ctr_object* ctr_string_eq(ctr_object* myself, ctr_argument* argumentList) {
	if (argumentList->object->value.svalue->vlen != myself->value.svalue->vlen) {
		return ctr_build_bool(0);
	}
	return ctr_build_bool((strncmp(argumentList->object->value.svalue->value, myself->value.svalue->value, myself->value.svalue->vlen)==0));
}

/**
 * [String] != [other]
 *
 * Returns True if the other string is not the same (in bytes).
 */
ctr_object* ctr_string_neq(ctr_object* myself, ctr_argument* argumentList) {
	if (argumentList->object->value.svalue->vlen != myself->value.svalue->vlen) {
		return ctr_build_bool(1);
	}
	return ctr_build_bool(!(strncmp(argumentList->object->value.svalue->value, myself->value.svalue->value, myself->value.svalue->vlen)==0));
}

/**
 * [String] length
 *
 * Returns the length of the string in symbols.
 * This message is UTF-8 unicode aware. A 4 byte character will be counted as ONE.
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
	dest = calloc(sizeof(char), (n1 + n2));
	memcpy(dest, myself->value.svalue->value, n1);
	memcpy(dest+n1, strObject->value.svalue->value, n2);
	newString = ctr_build_string(dest, (n1 + n2));
	return newString;	
}

/**
 * [String] from: [position] to: [destination]
 *
 * Returns a portion of a string defined by from-to values.
 * This message is UTF-8 unicode aware.
 *
 * Usage:
 * 
 * 'hello' from: 2 to: 3. #ll
 */
ctr_object* ctr_string_fromto(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* fromPos = ctr_internal_cast2number(argumentList->object);
	ctr_object* toPos = ctr_internal_cast2number(argumentList->next->object);
	long len = myself->value.svalue->vlen;
	long a = (fromPos->value.nvalue);
	long b = (toPos->value.nvalue); 
	long t;
	long ua, ub;
	char* dest;
	ctr_object* newString;
	if (b == a) return ctr_build_string("",0);
	if (a > b) {
		t = a; a = b; b = t;
	}
	if (a > len) return ctr_build_string("", 0);
	if (b > len) b = len;
	if (a < 0) a = 0;
	if (b < 0) return ctr_build_string("", 0);
	ua = getBytesUtf8(myself->value.svalue->value, 0, a);
	ub = getBytesUtf8(myself->value.svalue->value, ua, ((b - a)));
	dest = malloc(ub * sizeof(char));
	memcpy(dest, (myself->value.svalue->value) + ua, ub);
	newString = ctr_build_string(dest,ub);
	return newString;
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
 * 'hello' from: 2 length: 3. #llo
 */
ctr_object* ctr_string_from_length(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* fromPos = ctr_internal_cast2number(argumentList->object);
	ctr_object* length = ctr_internal_cast2number(argumentList->next->object);
	long len = myself->value.svalue->vlen;
	long a = (fromPos->value.nvalue);
	long b = (length->value.nvalue);
	long ua, ub;
	char* dest;
	ctr_object* newString;
	if (b == 0) return ctr_build_string("",0);
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
	dest = malloc(ub * sizeof(char));
	memcpy(dest, (myself->value.svalue->value) + ua, ub);
	newString = ctr_build_string(dest,ub);
	return newString;
}

/**
 * [String] skip: [number]
 *
 * Returns a string without the first X characters.
 */
ctr_object* ctr_string_skip(ctr_object* myself, ctr_argument* argumentList) {
	ctr_argument* argument1;
	ctr_argument* argument2;
	if (myself->value.svalue->vlen < argumentList->object->value.nvalue) return ctr_build_string("",0);
	argument1 = CTR_CREATE_ARGUMENT();
	argument2 = CTR_CREATE_ARGUMENT();
	argument1->object = argumentList->object;
	argument1->next = argument2;
	argument2->object = ctr_build_number_from_float(myself->value.svalue->vlen - argumentList->object->value.nvalue);
	return ctr_string_from_length(myself, argument1);
}


/**
 * [String] at: [position]
 *
 * Returns the character at the specified position (UTF8 aware).
 *
 * Usage:
 * 
 * ('hello' at: 2). #l
 */
ctr_object* ctr_string_at(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* fromPos = ctr_internal_cast2number(argumentList->object);
	long a = (fromPos->value.nvalue);
	long ua = getBytesUtf8(myself->value.svalue->value, 0, a);
	long ub = getBytesUtf8(myself->value.svalue->value, ua, 1);
	ctr_object* newString;
	char* dest = malloc(ub * sizeof(char));
	memcpy(dest, (myself->value.svalue->value) + ua, ub);
	newString = ctr_build_string(dest,ub);
	free(dest);
	return newString;
}

/**
 * [String] byteAt: [position]
 *
 * Returns the byte at the specified position (in bytes).
 *
 * Usage:
 * ('abc' byteAt: 1). #98
 */
ctr_object* ctr_string_byte_at(ctr_object* myself, ctr_argument* argumentList) {
	char x;
	ctr_object* fromPos = ctr_internal_cast2number(argumentList->object);
	long a = (fromPos->value.nvalue);
	long len = myself->value.svalue->vlen;
	if (a > len) return CtrStdNil;
	if (a < 0) return CtrStdNil;
	x = (char) *(myself->value.svalue->value + a);
	return ctr_build_number_from_float((double)x);
}

/**
 * [String] indexOf: [subject]
 *
 * Returns the index (character number, not the byte!) of the
 * needle in the haystack.
 * 
 * Usage:
 *
 * 'find the needle' indexOf: 'needle'. #9
 *
 */
ctr_object* ctr_string_index_of(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* sub = ctr_internal_cast2string(argumentList->object);
	long hlen = myself->value.svalue->vlen;
	long nlen = sub->value.svalue->vlen;
	uintptr_t byte_index;
	ctr_size uchar_index;
	char* p = ctr_internal_memmem(myself->value.svalue->value, hlen, sub->value.svalue->value, nlen, 0);
	if (p == NULL) return ctr_build_number_from_float((ctr_number)-1);
	byte_index = (uintptr_t) p - (uintptr_t) (myself->value.svalue->value);
	uchar_index = ctr_getutf8len(myself->value.svalue->value, byte_index);
	return ctr_build_number_from_float((ctr_number) uchar_index);
}

/**
 * [String] up
 * 
 * Returns a new uppercased version of the string.
 * Note that this is just basic ASCII case functionality, this should only
 * be used for internal keys and as a basic utility function. This function
 * DOES NOT WORK WITH UTF8 characters !
 */
ctr_object* ctr_string_to_upper(ctr_object* myself, ctr_argument* argumentList) {
       char* str = myself->value.svalue->value;
       size_t  len = myself->value.svalue->vlen;
       char* tstr = malloc(len * sizeof(char));
       int i=0;
       for(i =0; i < len; i++) {
               tstr[i] = toupper(str[i]);
       }
       return ctr_build_string(tstr, len);
}


/**
 * [String] low
 * 
 * Returns a new lowercased version of the string.
 * Note that this is just basic ASCII case functionality, this should only
 * be used for internal keys and as a basic utility function. This function
 * DOES NOT WORK WITH UTF8 characters !
 */
ctr_object* ctr_string_to_lower(ctr_object* myself, ctr_argument* argumentList) {
       char* str = myself->value.svalue->value;
       size_t len = myself->value.svalue->vlen;
       char* tstr = malloc(len * sizeof(char));
       int i=0;
       for(i =0; i < len; i++) {
               tstr[i] = tolower(str[i]);
       }
       return ctr_build_string(tstr, len);
}

/**
 * [String] lastIndexOf: [subject]
 *
 * Returns the index (character number, not the byte!) of the
 * needle in the haystack.
 * 
 * Usage:
 * 
 * 'find the needle' lastIndexOf: 'needle'. #9
 */
ctr_object* ctr_string_last_index_of(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* sub = ctr_internal_cast2string(argumentList->object);
	long hlen = myself->value.svalue->vlen;
	long nlen = sub->value.svalue->vlen;
	ctr_size uchar_index;
	ctr_size byte_index;
	char* p = ctr_internal_memmem(myself->value.svalue->value, hlen, sub->value.svalue->value, nlen, 1);
	if (p == NULL) return ctr_build_number_from_float((float)-1);
	byte_index = (ctr_size) ( (uintptr_t) p - (uintptr_t) (myself->value.svalue->value) );
	uchar_index = ctr_getutf8len(myself->value.svalue->value, byte_index);
	return ctr_build_number_from_float((float) uchar_index);
}

/**
 * [String] replace: [string] with: [other]
 *
 * Replaces needle with replacement in original string and returns
 * the result as a new string object.
 *
 * Usage:
 *
 * 'LiLo BootLoader' replace: 'L' with: 'l'. #lilo Bootloader
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
	dest = (char*) malloc(dlen*sizeof(char));
	odest = dest;
	if (nlen == 0 || hlen == 0) {
		return ctr_build_string(src, hlen);
	}
	while(1) {
		p = ctr_internal_memmem(src, hlen, ndl, nlen, 0);
		if (p == NULL) break;
		d = (dest - odest);
		if ((dlen - nlen + rlen)>dlen) {
			dlen = (dlen - nlen + rlen);
			odest = (char*) realloc(odest, dlen * sizeof(char));
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
	return ctr_build_string(odest, dlen);
}

/**
 * [String] trim
 *
 * Trims a string. Removes surrounding white space characters
 * from string and returns the result as a new string object.
 *
 * Usage:
 *
 * ' hello ' trim. #hello
 *
 */
ctr_object* ctr_string_trim(ctr_object* myself, ctr_argument* argumentList) {
	char* str = myself->value.svalue->value;
	long  len = myself->value.svalue->vlen;
	long i, begin, end, tlen;
	char* tstr;
	if (len == 0) return ctr_build_string("", 0);
	i = 0;
	while(i < len && isspace(*(str+i))) i++;
	begin = i;
	i = len - 1;
	while(i > begin && isspace(*(str+i))) i--;
	end = i + 1;
	tlen = (end - begin);
	tstr = malloc(tlen * sizeof(char));
	memcpy(tstr, str+begin, tlen);
	return ctr_build_string(tstr, tlen);
}


/**
 * [String] ltrim
 * 
 * Removes all the whitespace at the left side of the string.
 */
ctr_object* ctr_string_ltrim(ctr_object* myself, ctr_argument* argumentList) {
	char* str = myself->value.svalue->value;
	long  len = myself->value.svalue->vlen;
	long i = 0, begin;
	long tlen;
	char* tstr;
	if (len == 0) return ctr_build_string("", 0);
	while(i < len && isspace(*(str+i))) i++;
	begin = i;
	i = len - 1;
	tlen = (len - begin);
	tstr = malloc(tlen * sizeof(char));
	memcpy(tstr, str+begin, tlen);
	return ctr_build_string(tstr, tlen);
}

/**
 * [String] rtrim
 * 
 * Removes all the whitespace at the right side of the string.
 */
ctr_object* ctr_string_rtrim(ctr_object* myself, ctr_argument* argumentList) {
	char* str = myself->value.svalue->value;
	long  len = myself->value.svalue->vlen;
	long i = 0, end, tlen;
	char* tstr;
	if (len == 0) return ctr_build_string("", 0);
	i = len - 1;
	while(i > 0 && isspace(*(str+i))) i--;
	end = i + 1;
	tlen = end;
	tstr = malloc(tlen * sizeof(char));
	memcpy(tstr, str, tlen);
	return ctr_build_string(tstr, tlen);
}

/**
 * [String] toNumber
 *
 * Converts string to a number.
 */
ctr_object* ctr_string_to_number(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_cast2number(myself);
}

/**
 * [String] toBoolean
 *
 * Converts string to boolean
 */
ctr_object* ctr_string_to_boolean(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_cast2bool(myself);
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
	ctr_argument* arg;
	char* elem;
	ctr_object* arr = ctr_array_new(CtrStdArray, NULL);
	long i;
	long j = 0;
	char* buffer = malloc(sizeof(char)*len);
	for(i=0; i<len; i++) {
		buffer[j] = str[i];
		j++;
		if (ctr_internal_memmem(buffer, j, dstr, dlen, 0)!=NULL) {
			elem = malloc(sizeof(char)*(j-dlen));
			memcpy(elem,buffer,j-dlen);
			arg = malloc(sizeof(ctr_argument));
			arg->object = ctr_build_string(elem, j-dlen);
			ctr_array_push(arr, arg);
			free(arg);
			free(elem);
			j=0;
		}
	}
	if (j>0) {
		elem = malloc(sizeof(char)*j);
		memcpy(elem,buffer,j);
		arg = malloc(sizeof(ctr_argument));
		arg->object = ctr_build_string(elem, j);
		ctr_array_push(arr, arg);
		free(arg);
		free(elem);
	}
	free(buffer);
	return arr;
}

/**
 * [String] htmlEscape
 * 
 * Escapes HTML chars.
 */

ctr_object* ctr_string_html_escape(ctr_object* myself, ctr_argument* argumentList)  {
	char* str = myself->value.svalue->value;
	long  len = myself->value.svalue->vlen;
	char* tstr;
	long i=0;
	long j=0;
	long k=0;
	long rlen;
	long tlen = 0;
	long tag_len = 0;
	long tag_rlen = 0;
	char* replacement;
	for(i =0; i < len; i++) {
		char c = str[i];
		switch(c) {
			case '<':
				tag_len += 4;
				tag_rlen += 1;
				break;
			case '>':
				tag_len += 4;
				tag_rlen += 1;
				break;
			case '&':
				tag_len += 5;
				tag_rlen += 1;
				break;
			case '"':
				tag_len += 6;
				tag_rlen += 1;
				break;
			case '\'':
				tag_len += 6;
				tag_rlen += 1;
				break;
			default:
				break;
		}
	}
	tlen = len + tag_len - tag_rlen;
	tstr = malloc(tlen * sizeof(char));
	for(i = 0; i < len; i++) {
		char c = str[i];
		switch (c) {
			case '<':
				replacement = "&lt;";
				rlen = 4;
				for(j=0; j<rlen; j++) tstr[k++]=replacement[j];
				break;
			case '>':
				replacement = "&gt;";
				rlen = 4;
				for(j=0; j<rlen; j++) tstr[k++]=replacement[j];
				break;
			case '&':
				replacement = "&amp;";
				rlen = 5;
				for(j=0; j<rlen; j++) tstr[k++]=replacement[j];
				break;
			case '"':
				replacement = "&quot;";
				rlen = 6;
				for(j=0; j<rlen; j++) tstr[k++]=replacement[j];
				break;
			case '\'':
				replacement = "&apos;";
				rlen = 6;
				for(j=0; j<rlen; j++) tstr[k++]=replacement[j];
				break;
			default:
				tstr[k++] = str[i];
				break;
		}
	}
	return ctr_build_string(tstr, tlen);
}

/**
 * Block
 *
 * Literal: 
 *
 * { parameters here... | code here... }
 * {\ code without parameters... }
 *
 * Examples:
 *
 * {\ Pen write: 'a simple code block'. } run.
 * { param | Pen write: param. } applyTo: 'write this!'.
 * { a b | ^ a + b. } applyTo: 1 and: 2.
 * { a b c | ^ a + b + c. } applyTo: 1 and: 2 and: 3.
 * 
 */
ctr_object* ctr_build_block(ctr_tnode* node) {
	ctr_object* codeBlockObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTBLOCK);
	codeBlockObject->value.block = node;
	codeBlockObject->link = CtrStdBlock;
	return codeBlockObject;
}

/**
 * [Block] applyTo: [object]
 *
 * Runs a block of code using the specified object as a parameter.
 */
ctr_object* ctr_block_run(ctr_object* myself, ctr_argument* argList, ctr_object* my) {
	ctr_object* result;
	ctr_tnode* node = myself->value.block;
	ctr_tlistitem* codeBlockParts = node->nodes;
	ctr_tnode* codeBlockPart1 = codeBlockParts->node;
	ctr_tnode* codeBlockPart2 = codeBlockParts->next->node;
	ctr_tlistitem* parameterList = codeBlockPart1->nodes;
	ctr_tnode* parameter;
	ctr_object* a;
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
	ctr_assign_value_to_local(ctr_build_string("me",2), my);
	ctr_assign_value_to_local(ctr_build_string("thisBlock",9), myself); /* otherwise running block may get gc'ed. */
	result = ctr_cwlk_run(codeBlockPart2);
	if (result == NULL) result = my;
	ctr_close_context();
	if (CtrStdError != NULL && CtrStdError != CtrStdBreak && CtrStdError != CtrStdContinue) {
		ctr_object* catchBlock = malloc(sizeof(ctr_object));
		catchBlock = ctr_internal_object_find_property(myself, ctr_build_string("catch",5), 0);
		if (catchBlock != NULL) {
			ctr_argument* a = CTR_CREATE_ARGUMENT();
			a->object = CtrStdError;
			CtrStdError = NULL;
			ctr_block_run(catchBlock, a, my);
			result = myself;
		}
	}
	return result;
}

/**
 * [Block] times: [Number]
 *
 * Runs the specified code block N times.
 *
 * Usage:
 * { ... } * 7.
 */
ctr_object* ctr_block_times(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* indexNumber;
	ctr_object* block = myself;
	ctr_argument* arguments;
	int t;
	int i;
	if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK) { printf("Expected code block."); exit(1); }
	block->info.sticky = 1;
	t = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	for(i=0; i<t; i++) {
		indexNumber = ctr_build_number_from_float((ctr_number) i);
		arguments = CTR_CREATE_ARGUMENT();
		arguments->object = indexNumber;
		ctr_block_run(block, arguments, block);
		if (CtrStdError == CtrStdContinue) CtrStdError = NULL; /* consume continue */
		if (CtrStdError) break;
	}
	if (CtrStdError == CtrStdBreak) CtrStdError = NULL; /* consume break */
	block->info.mark = 0;
	block->info.sticky = 0;
	return myself;
}

/**
 * [Block] whileTrue: [block]
 *
 * Runs a block of code, depending on the outcome runs the other block
 * as long as the result of the first one equals boolean True.
 */
ctr_object* ctr_block_while_true(ctr_object* myself, ctr_argument* argumentList) {
	while (1 && !CtrStdError) {
		ctr_object* result = ctr_internal_cast2bool(ctr_block_run(myself, argumentList, myself));
		if (result->value.bvalue == 0 || CtrStdError) break;
		ctr_block_run(argumentList->object, argumentList, argumentList->object);
		if (CtrStdError == CtrStdContinue) CtrStdError = NULL; /* consume continue */
	}
	if (CtrStdError == CtrStdBreak) CtrStdError = NULL; /* consume break */
	return myself;
}

/**
 * [Block] whileFalse: [block]
 *
 * Runs a block of code, depending on the outcome runs the other block
 * as long as the result of the first one equals boolean False.
 */
ctr_object* ctr_block_while_false(ctr_object* myself, ctr_argument* argumentList) {
	while (1 && !CtrStdError) {
		ctr_object* result = ctr_internal_cast2bool(ctr_block_run(myself, argumentList, myself));
		if (result->value.bvalue == 1 || CtrStdError) break;
		ctr_block_run(argumentList->object, argumentList, argumentList->object);
		if (CtrStdError == CtrStdContinue) CtrStdError = NULL; /* consume continue */
	}
	if (CtrStdError == CtrStdBreak) CtrStdError = NULL; /* consume break */
	return myself;
}

/**
 * [Block] run
 *
 * Runs a block
 */
ctr_object* ctr_block_runIt(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* result;
	result = ctr_block_run(myself, argumentList, myself);
	if (CtrStdError == CtrStdBreak || CtrStdError == CtrStdContinue) CtrStdError = NULL; /* consume break */
	return result;
}

/**
 * [Block] set: [name] value: [object]
 *
 * Sets a variable in a closure.
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
 * Example:
 * 
 * {\
 *   thisBlock error: 'oops!'.
 * } catch: { errorMessage |
 *   Pen write: errorMessage.	
 * }, run.
 */
ctr_object* ctr_block_error(ctr_object* myself, ctr_argument* argumentList) {
	CtrStdError = argumentList->object;
	return myself;
}

/**
 * [Block] catch: [otherBlock]
 * 
 * Associates an error clause to a block.
 * If an error (exception) occurs within the block this block will be
 * executed.
 * 
 * Example:
 * 
 * #Raise error on division by zero.
 * {\ 
 *    var z := 4 / 0.
 * } catch: { errorMessage | 
 *    Pen write: e, brk.
 * }, run.
 */
ctr_object* ctr_block_catch(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* catchBlock = argumentList->object;
	ctr_internal_object_delete_property(myself, ctr_build_string("catch",5),0);
	ctr_internal_object_add_property(myself, ctr_build_string("catch",5), catchBlock, 0);
	return myself;
}


