
/**
 * Builds a Nil object.
 *
 * Literal form: Nil
 */
ctr_object* ctr_build_nil() {	
	return Nil;
}

/**
 * Root Object
 * This is the base object, the parent of all other objects.
 * It contains essential object oriented programming features.
 */

/**
 * Creates a new instance of the Root Object.
 */ 
ctr_object* ctr_object_make() {
	ctr_object* objectInstance = NULL;
	objectInstance = ctr_internal_create_object(OTOBJECT);
	objectInstance->link = Object;
	return objectInstance;
}

/**
 * ObjectType
 *
 * Returns a string representation of the type of object.
 */
ctr_object* ctr_object_type(ctr_object* myself, ctr_argument* argumentList) {
	int type = myself->info.type;
	if (type == OTNIL) {
		return ctr_build_string_from_cstring("Nil\0");
	} else if (type == OTBOOL) {
		return ctr_build_string_from_cstring("Boolean\0");
	} else if (type == OTNUMBER) {
		return ctr_build_string_from_cstring("Number\0");
	} else if (type == OTSTRING) {
		return ctr_build_string_from_cstring("String\0");
	} else if (type == OTBLOCK || type == OTNATFUNC) {
		return ctr_build_string_from_cstring("Block\0");
	}
	return ctr_build_string_from_cstring("Object\0");
}

/**
 * Equals
 *
 * Tests whether the current instance is the same as
 * the argument.
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
 * OnMessageDoAction
 *
 * Makes the object respond to a new kind of message.
 *
 * Usage:
 * object on: 'greet' do {\ ... }.
 */
ctr_object* ctr_object_on_do(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* methodName = argumentList->object;
	if (methodName->info.type != OTSTRING) {
		error = ctr_build_string_from_cstring("Expected on: argument to be of type string.");
		return myself;
	}
	ctr_argument* nextArgument = argumentList->next;
	ctr_object* methodBlock = nextArgument->object;
	if (methodBlock->info.type != OTBLOCK) {
		error = ctr_build_string_from_cstring("Expected argument do: to be of type block.");
		return myself;
	}
	ctr_internal_object_add_property(myself, methodName, methodBlock, 1);
	return myself;
}

/**
 * OverrideDo
 *
 * Overrides the response of an object, replacing the
 * code block with the one specified in the second parameter.
 *
 * You can still invoke the old response by sending the message:
 * overridden-x where x is the original message.
 *
 * Usage:
 * object override: 'test' do: {\ ... }.
 * 
 */
ctr_object* ctr_object_override_does(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* methodName = argumentList->object;
	if (methodName->info.type != OTSTRING) {
		error = ctr_build_string_from_cstring("Expected on: argument to be of type string.");
		return myself;
	}
	ctr_argument* nextArgument = argumentList->next;
	ctr_object* methodBlock = nextArgument->object;
	if (methodBlock->info.type != OTBLOCK) {
		error = ctr_build_string_from_cstring("Expected argument do: to be of type block.");
		return myself;
	}
	ctr_object* overriddenMethod = ctr_internal_object_find_property(myself, methodName, 1);
	if (overriddenMethod == NULL) {
		error = ctr_build_string_from_cstring("Cannot override, original response not found.");
		return myself;
	}
	char* superMethodNameString = malloc(sizeof(char) * (methodName->value.svalue->vlen + 11));
	memcpy(superMethodNameString, "overridden-", (sizeof(char) * 11));
	memcpy(superMethodNameString + (sizeof(char)*11), methodName->value.svalue->value, methodName->value.svalue->vlen);
	ctr_object* superMethodKey = ctr_build_string(superMethodNameString, (methodName->value.svalue->vlen + 11));
	ctr_internal_object_delete_property(myself, methodName, 1);
	ctr_internal_object_add_property(myself, superMethodKey, overriddenMethod, 1);
	ctr_internal_object_add_property(myself, methodName, methodBlock, 1);
	return myself;
}


/**
 * Default respond-to implemention, does nothing.
 */
ctr_object* ctr_object_respond(ctr_object* myself, ctr_argument* argumentList) {
	return myself;
}

/**
 * Uses the specified object as blueprint or prototype.
 * If a message can't be answered by an object it will send the message
 * to its prototype (and so on) before passing the message to the
 * generic respond message handler.
 * 
 * Usage:
 * 
 * object new basedOn: parentObject.
 */
ctr_object* ctr_object_basedOn(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* other = argumentList->object;
	if (other == myself) {
		error = ctr_build_string_from_cstring("Circular prototype.");
		return myself;
	}
	myself->link = other;
	return myself;
}

/**
 * Booleans.
 * 
 * Literal form: True False
 */
ctr_object* ctr_build_bool(int truth) {
	ctr_object* boolObject = ctr_internal_create_object(OTBOOL);
	if (truth) boolObject->value.bvalue = 1; else boolObject->value.bvalue = 0;
	boolObject->info.type = OTBOOL;
	boolObject->link = BoolX;
	return boolObject;
}

/**
 * ifTrue
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
		return ctr_block_run(codeBlock, arguments, myself);
	}
	return myself;
}

/**
 * ifFalse
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
		return ctr_block_run(codeBlock, arguments, myself);
	}
	return myself;
}

/**
 * Opposite
 *
 * Returns the opposite of the current value.
 *
 * Usage:
 * Yes = No opposite.
 *
 */
ctr_object* ctr_bool_opposite(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(!myself->value.bvalue);
}

/**
 * BooleanAnd
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
 * BooleanOr
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
 * BooleanXOR
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
 * BuildNumber
 *
 * Creates a number from a C string (0 terminated).
 * Internal use only.
 */
ctr_object* ctr_build_number(char* n) {
	ctr_object* numberObject = ctr_internal_create_object(OTNUMBER);
	numberObject->value.nvalue = atof(n);
	numberObject->link = Number;
	return numberObject;
}

/**
 * BuildNumberFromFloat
 *
 * Creates a number object from a float.
 * Internal use only.
 */
ctr_object* ctr_build_number_from_float(double f) {
	ctr_object* numberObject = ctr_internal_create_object(OTNUMBER);
	numberObject->value.nvalue = f;
	numberObject->link = Number;
	return numberObject;
}

/**
 * Number Comparison Operators
 *
 * Implementation of >, <, >=, <=, == and !=
 */
ctr_object* ctr_number_higherThan(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue > otherNum->value.nvalue));
}

ctr_object* ctr_number_higherEqThan(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue >= otherNum->value.nvalue));
}

ctr_object* ctr_number_lowerThan(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue < otherNum->value.nvalue));
}

ctr_object* ctr_number_lowerEqThan(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue <= otherNum->value.nvalue));
}

ctr_object* ctr_number_eq(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool(myself->value.nvalue == otherNum->value.nvalue);
}

ctr_object* ctr_number_neq(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool(myself->value.nvalue != otherNum->value.nvalue);
}

/**
 * BetweenAnd
 *
 * Returns True if the number instance has a value between the two
 * specified values.
 *
 * Usage:
 * q between: x and: y
 */
ctr_object* ctr_number_between(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_argument* nextArgumentItem = argumentList->next;
	ctr_object* nextArgument = ctr_internal_cast2number(nextArgumentItem->object);
	return ctr_build_bool((myself->value.nvalue >=  otherNum->value.nvalue) && (myself->value.nvalue <= nextArgument->value.nvalue));
}


/**
 * Arithmetic
 * 
 * The following functions implement basic arithmetic operations:
 * 
 * + - / * inc dec mul div
 */
ctr_object* ctr_string_concat(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_add(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = argumentList->object;
	if (otherNum->info.type == OTSTRING) {
		ctr_object* strObject = ctr_internal_create_object(OTSTRING);
		strObject = ctr_internal_cast2string(myself);
		ctr_argument* newArg = CTR_CREATE_ARGUMENT();
		newArg->object = otherNum;
		return ctr_string_concat(strObject, newArg);
	}
ctr_number a = myself->value.nvalue;
ctr_number b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a+b));
}


ctr_object* ctr_number_inc(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	myself->value.nvalue += otherNum->value.nvalue;
	return myself;
}

ctr_object* ctr_number_minus(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
ctr_number a = myself->value.nvalue;
ctr_number b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a-b));
}

ctr_object* ctr_number_dec(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	myself->value.nvalue -= otherNum->value.nvalue;
	return myself;
}

ctr_object* ctr_number_multiply(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
ctr_number a = myself->value.nvalue;
ctr_number b = otherNum->value.nvalue;
	return ctr_build_number_from_float(a*b);
}

ctr_object* ctr_number_mul(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	myself->value.nvalue *= otherNum->value.nvalue;
	return myself;
}

ctr_object* ctr_number_divide(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
ctr_number a = myself->value.nvalue;
ctr_number b = otherNum->value.nvalue;
	if (b == 0) {
		error = ctr_build_string_from_cstring("Division by zero.");
		return myself;
	}
	return ctr_build_number_from_float((a/b));
}

ctr_object* ctr_number_div(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	if (otherNum->value.nvalue == 0) {
		error = ctr_build_string_from_cstring("Division by zero.");
		return myself;
	}
	myself->value.nvalue /= otherNum->value.nvalue;
	return myself;
}

ctr_object* ctr_number_modulo(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
ctr_number a = myself->value.nvalue;
ctr_number b = otherNum->value.nvalue;
	if (b == 0) {
		error = ctr_build_string_from_cstring("Division by zero.");
		return myself;
	}
	return ctr_build_number_from_float(fmod(a,b));
}

ctr_object* ctr_number_pow(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
ctr_number a = myself->value.nvalue;
ctr_number b = otherNum->value.nvalue;
	return ctr_build_number_from_float(pow(a,b));
}

ctr_object* ctr_number_max(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
ctr_number a = myself->value.nvalue;
ctr_number b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a >= b) ? a : b);
}

ctr_object* ctr_number_min(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
ctr_number a = myself->value.nvalue;
ctr_number b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a <= b) ? a : b);
}

/**
 * Factorial
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
 * Some basic math functions: floor, ceil, round, abs, sqrt, pow, sin, cos
 * tan, atan and log.
 */
ctr_object* ctr_number_floor(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(floor(myself->value.nvalue));
}

ctr_object* ctr_number_ceil(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(ceil(myself->value.nvalue));
}

ctr_object* ctr_number_round(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(round(myself->value.nvalue));
}

ctr_object* ctr_number_abs(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(fabs(myself->value.nvalue));
}

ctr_object* ctr_number_sqrt(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(sqrt(myself->value.nvalue));
}

ctr_object* ctr_number_exp(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(exp(myself->value.nvalue));
}

ctr_object* ctr_number_sin(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(sin(myself->value.nvalue));
}

ctr_object* ctr_number_cos(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(cos(myself->value.nvalue));
}

ctr_object* ctr_number_tan(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(tan(myself->value.nvalue));
}

ctr_object* ctr_number_atan(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(atan(myself->value.nvalue));
}

ctr_object* ctr_number_log(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(log(myself->value.nvalue));
}

/**
 * NumberToString
 *
 * Wrapper for cast function.
 */
ctr_object* ctr_number_to_string(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_cast2string(myself);
}

/**
 * NumberToBoolean
 *
 * Wrapper for cast function.
 */
ctr_object* ctr_number_to_boolean(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_cast2bool(myself);
}

/**
 * BooleanToNumber
 *
 * Wrapper for cast function.
 * Returns 0 if boolean is False and 1 otherwise.
 */
ctr_object* ctr_bool_to_number(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float( (float) myself->value.bvalue );
}

/**
 * Times
 *
 * Runs the specified code block N times.
 *
 * Usage:
 * 7 times: { ... }.
 */
ctr_object* ctr_number_times(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* block = argumentList->object;
	if (block->info.type != OTBLOCK) { printf("Expected code block."); exit(1); }
	block->info.sticky = 1; //mark as sticky
	int t = myself->value.nvalue;
	int i;
	for(i=0; i<t; i++) {
		char* nstr = (char*) calloc(20, sizeof(char));
		snprintf(nstr, 20, "%d", i);
		ctr_object* indexNumber = ctr_build_number(nstr);
		ctr_argument* arguments = CTR_CREATE_ARGUMENT();
		arguments->object = indexNumber;
		ctr_block_run(block, arguments, myself);
	}
	block->info.mark = 0;
	block->info.sticky = 0;
	return myself;
}

/**
 * BuildString
 *
 * Creates a Citrine String object.
 */
ctr_object* ctr_build_string(char* stringValue, long size) {
	ctr_object* stringObject = ctr_internal_create_object(OTSTRING);
	if (size != 0) {
		stringObject->value.svalue->value = malloc(size*sizeof(char));
		memcpy(stringObject->value.svalue->value, stringValue, size);
	}
	stringObject->value.svalue->vlen = size;
	stringObject->link = TextString;
	return stringObject;
}

/**
 * BuildStringFromCString
 *
 * Creates a Citrine String from a 0 terminated C String.
 */
ctr_object* ctr_build_string_from_cstring(char* cstring) {
	return ctr_build_string(cstring, strlen(cstring));
}

/**
 * StringBytes
 *
 * Returns the number of bytes in a string.
 */
ctr_object* ctr_string_bytes(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float((float)myself->value.svalue->vlen);
}

/**
 * StringEquality
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
 * StringNonEquality
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
 * StringLength
 *
 * Returns the length of the string in symbols.
 * This message is UTF-8 unicode aware. A 4 byte character will be counted as ONE.
 */
ctr_object* ctr_string_length(ctr_object* myself, ctr_argument* argumentList) {
	long n = getutf8len(myself->value.svalue->value, myself->value.svalue->vlen);
	char* str = calloc(100, sizeof(char));
	snprintf(str, 100, "%lu", n);
	return ctr_build_number(str);
}

/**
 * StringConcat
 *
 * Appends other string to self and returns the resulting
 * string as a new object.
 */
ctr_object* ctr_string_concat(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* strObject = ctr_internal_create_object(OTSTRING);
	strObject = ctr_internal_cast2string(argumentList->object);
	long n1 = myself->value.svalue->vlen;
	long n2 = strObject->value.svalue->vlen;
	char* dest = calloc(sizeof(char), (n1 + n2));
	memcpy(dest, myself->value.svalue->value, n1);
	memcpy(dest+n1, strObject->value.svalue->value, n2);
	ctr_object* newString = ctr_build_string(dest, (n1 + n2));
	return newString;	
}

/**
 * StringFromTo
 *
 * Returns a portion of a string defined by from-to values.
 * This message is UTF-8 unicode aware.
 *
 * Usage:
 * 'hello' from: 2 to: 3. #ll
 */
ctr_object* ctr_string_fromto(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* fromPos = ctr_internal_cast2number(argumentList->object);
	ctr_object* toPos = ctr_internal_cast2number(argumentList->next->object);
	long len = myself->value.svalue->vlen;
	long a = (fromPos->value.nvalue);
	long b = (toPos->value.nvalue); 
	long t;
	if (b == a) return ctr_build_string("",0);
	if (a > b) {
		t = a; a = b; b = t;
	}
	if (a > len) return ctr_build_string("", 0);
	if (b > len) b = len;
	if (a < 0) a = 0;
	if (b < 0) return ctr_build_string("", 0);
	long ua = getBytesUtf8(myself->value.svalue->value, 0, a);
	long ub = getBytesUtf8(myself->value.svalue->value, ua, ((b - a)));
	char* dest = malloc(ub * sizeof(char));
	memcpy(dest, (myself->value.svalue->value) + ua, ub);
	ctr_object* newString = ctr_build_string(dest,ub);
	return newString;
}

/**
 * StringFromLength
 *
 * Returns a portion of a string defined by from 
 * and length values.
 * This message is UTF-8 unicode aware.
 *
 * Usage:
 * 'hello' from: 2 length: 3. #llo
 */
ctr_object* ctr_string_from_length(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* fromPos = ctr_internal_cast2number(argumentList->object);
	ctr_object* length = ctr_internal_cast2number(argumentList->next->object);
	long len = myself->value.svalue->vlen;
	long a = (fromPos->value.nvalue);
	long b = (length->value.nvalue);
	if (b == 0) return ctr_build_string("",0);
	if (b < 0) {
		a = a + b;
		b = abs(b);
	}
	if (a < 0) a = 0;
	if (a > len) a = len;
	if ((a + b)>len) b = len - a;
	if ((a + b)<0) b = b - a;
	long ua = getBytesUtf8(myself->value.svalue->value, 0, a);
	long ub = getBytesUtf8(myself->value.svalue->value, ua, b);
	char* dest = malloc(ub * sizeof(char));
	memcpy(dest, (myself->value.svalue->value) + ua, ub);
	ctr_object* newString = ctr_build_string(dest,ub);
	return newString;
}

/**
 * StringCharacterAt
 *
 * Returns the character at the specified position (UTF8 aware).
 *
 * Usage:
 * ('hello' at: 2). #l
 */
ctr_object* ctr_string_at(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* fromPos = ctr_internal_cast2number(argumentList->object);
	long a = (fromPos->value.nvalue);
	long ua = getBytesUtf8(myself->value.svalue->value, 0, a);
	long ub = getBytesUtf8(myself->value.svalue->value, ua, 1);
	char* dest = malloc(ub * sizeof(char));
	memcpy(dest, (myself->value.svalue->value) + ua, ub);
	ctr_object* newString = ctr_build_string(dest,ub);
	return newString;
}

/**
 * StringByteAt
 *
 * Returns the byte at the specified position (in bytes).
 *
 * Usage:
 * ('abc' byteAt: 1). #98
 */
ctr_object* ctr_string_byte_at(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* fromPos = ctr_internal_cast2number(argumentList->object);
	long a = (fromPos->value.nvalue);
	long len = myself->value.svalue->vlen;
	if (a > len) return Nil;
	if (a < 0) return Nil;
	char x = *(myself->value.svalue->value + a);
	return ctr_build_number_from_float((float)x);
}

/**
 * StringIndexOf
 *
 * Returns the index (character number, not the byte!) of the
 * needle in the haystack.
 * 
 * Usage:
 * 'find the needle' indexOf: 'needle'. #9
 *
 */
ctr_object* ctr_string_index_of(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* sub = ctr_internal_cast2string(argumentList->object);
	long hlen = myself->value.svalue->vlen;
	long nlen = sub->value.svalue->vlen;
	char* p = ctr_internal_memmem(myself->value.svalue->value, hlen, sub->value.svalue->value, nlen, 0);
	if (p == NULL) return ctr_build_number_from_float((float)-1);
	uintptr_t byte_index = (uintptr_t) p - (uintptr_t) (myself->value.svalue->value);
	uintptr_t uchar_index = getutf8len(myself->value.svalue->value, byte_index);
	return ctr_build_number_from_float((float) uchar_index);
}

/**
 * StringLastIndexOf
 *
 * Returns the index (character number, not the byte!) of the
 * needle in the haystack.
 * 
 * Usage:
 * 'find the needle' lastIndexOf: 'needle'. #9
 */
ctr_object* ctr_string_last_index_of(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* sub = ctr_internal_cast2string(argumentList->object);
	long hlen = myself->value.svalue->vlen;
	long nlen = sub->value.svalue->vlen;
	char* p = ctr_internal_memmem(myself->value.svalue->value, hlen, sub->value.svalue->value, nlen, 1);
	if (p == NULL) return ctr_build_number_from_float((float)-1);
	uintptr_t byte_index = (uintptr_t) p - (uintptr_t) (myself->value.svalue->value);
	uintptr_t uchar_index = getutf8len(myself->value.svalue->value, byte_index);
	return ctr_build_number_from_float((float) uchar_index);
}

/**
 * StringReplaceWith
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
	dest = (char*) malloc(dlen*sizeof(char));
	odest = dest;
	if (nlen == 0 || hlen == 0) {
		return ctr_build_string(src, hlen);
	}
	while(1) {
		p = memmem(src, hlen, ndl, nlen);
		if (p == NULL) break;
		long d = (dest - odest);
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
 * StringTrim
 *
 * Trims a string. Removes surrounding white space characters
 * from string and returns the result as a new string object.
 *
 * Usage:
 * ' hello ' trim. #hello
 *
 */
ctr_object* ctr_string_trim(ctr_object* myself, ctr_argument* argumentList) {
	char* str = myself->value.svalue->value;
	long  len = myself->value.svalue->vlen;
	if (len == 0) return ctr_build_string("", 0);
	long i = 0;
	while(i < len && isspace(*(str+i))) i++;
	long begin = i;
	i = len - 1;
	while(i > begin && isspace(*(str+i))) i--;
	long end = i + 1;
	long tlen = (end - begin);
	char* tstr = malloc(tlen * sizeof(char));
	memcpy(tstr, str+begin, tlen);
	return ctr_build_string(tstr, tlen);
}


/**
 * StringLeftTrim
 */
ctr_object* ctr_string_ltrim(ctr_object* myself, ctr_argument* argumentList) {
	char* str = myself->value.svalue->value;
	long  len = myself->value.svalue->vlen;
	if (len == 0) return ctr_build_string("", 0);
	long i = 0;
	while(i < len && isspace(*(str+i))) i++;
	long begin = i;
	i = len - 1;
	long tlen = (len - begin);
	char* tstr = malloc(tlen * sizeof(char));
	memcpy(tstr, str+begin, tlen);
	return ctr_build_string(tstr, tlen);
}

/**
 * StringRightTrim
 */
ctr_object* ctr_string_rtrim(ctr_object* myself, ctr_argument* argumentList) {
	char* str = myself->value.svalue->value;
	long  len = myself->value.svalue->vlen;
	if (len == 0) return ctr_build_string("", 0);
	long i = 0;
	i = len - 1;
	while(i > 0 && isspace(*(str+i))) i--;
	long end = i + 1;
	long tlen = end;
	char* tstr = malloc(tlen * sizeof(char));
	memcpy(tstr, str, tlen);
	return ctr_build_string(tstr, tlen);
}

/**
 * StringToNumber
 *
 * Wrapper for cast function.
 */
ctr_object* ctr_string_to_number(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_cast2number(myself);
}

/**
 * StringToBooleam
 *
 * Wrapper for cast function.
 */
ctr_object* ctr_string_to_boolean(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_cast2bool(myself);
}

/**
 * BooleanToString
 *
 * Simple cast function.
 */
ctr_object* ctr_bool_to_string(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_cast2string(myself);
}

/**
 * StringHTMLEscape
 *
 * Escapes HTML chars.
 */
ctr_object* ctr_string_html_escape(ctr_object* myself, ctr_argument* argumentList) {
	char* str = myself->value.svalue->value;
	long  len = myself->value.svalue->vlen;
	char* tstr = malloc(len * sizeof(char));
	long i=0;
	long j=0;
	long k=0;
	long rlen;
	long tlen = len;
	char* replacement;
	for(i =0; i < len; i++) {
		char c = str[i];
		if (c == '<') {
			replacement = "&lt;";
			rlen = 4;
			tlen += (rlen - 1);
			tstr = realloc(tstr, (tlen) * sizeof(char));
			for(j=0; j<rlen; j++) tstr[k+j]=replacement[j];
			k += rlen;
		} else if (c == '>') {
			replacement = "&gt;";
			rlen = 4;
			tlen += (rlen - 1);
			tstr = realloc(tstr, (tlen) * sizeof(char));
			for(j=0; j<rlen; j++) tstr[k+j]=replacement[j];
			k += rlen;
		} else if (c == '&') {
			replacement = "&amp;";
			rlen = 5;
			tlen += (rlen - 1);
			tstr = realloc(tstr, (tlen) * sizeof(char));
			for(j=0; j<rlen; j++) tstr[k+j]=replacement[j];
			k += rlen;
		} else if (c == '"') {
			replacement = "&quot;";
			rlen = 6;
			tlen += (rlen - 1);
			tstr = realloc(tstr, (tlen) * sizeof(char));
			for(j=0; j<rlen; j++) tstr[k+j]=replacement[j];
			k += rlen;
		}
		else { 
			tstr[k++] = str[i];
		}
	}
	return ctr_build_string(tstr, tlen);
}

/**
 * BlockRun
 *
 * Runs a block of code.
 */
ctr_object* ctr_block_run(ctr_object* myself, ctr_argument* argList, ctr_object* my) {
	ctr_object* result;
	tnode* node = myself->value.block;
	tlistitem* codeBlockParts = node->nodes;
	tnode* codeBlockPart1 = codeBlockParts->node;
	tnode* codeBlockPart2 = codeBlockParts->next->node;
	tlistitem* parameterList = codeBlockPart1->nodes;
	tnode* parameter;
	ctr_open_context();
	if (parameterList && parameterList->node) {
		parameter = parameterList->node;
		ctr_object* a;
		while(1) {
			if (parameter && argList->object) {
				a = argList->object;
				ctr_set(ctr_build_string(parameter->value, parameter->vlen), a);
			}
			if (!argList->next) break;
			argList = argList->next;
			if (!parameterList->next) break;
			parameterList = parameterList->next;
			parameter = parameterList->node;
		}
	}
	ctr_set(ctr_build_string("me",2), my);
	ctr_set(ctr_build_string("thisBlock",9), myself); //otherwise running block may get gc'ed.
	result = cwlk_run(codeBlockPart2);
	ctr_close_context();
	if (error != NULL) {
		ctr_object* catchBlock = malloc(sizeof(ctr_object));
		catchBlock = ctr_internal_object_find_property(myself, ctr_build_string("catch",5), 0);
		if (catchBlock != NULL) {
			ctr_argument* a = CTR_CREATE_ARGUMENT();
			a->object = error;
			error = NULL;
			ctr_block_run(catchBlock, a, my);
			result = myself;
		}
	}
	return result;
}

/**
 * BlockWhileTrue
 *
 * Runs a block of code, depending on the outcome runs the other block
 * as long as the result of the first one equals boolean True.
 */
ctr_object* ctr_block_while_true(ctr_object* myself, ctr_argument* argumentList) {
	while (1) {
		ctr_object* result = ctr_internal_cast2bool(ctr_block_run(myself, argumentList, myself));
		if (result->value.bvalue == 0) break;
		ctr_block_run(argumentList->object, argumentList, argumentList->object);
	}
	return myself;
}

/**
 * BlockWhileFalse
 *
 * Runs a block of code, depending on the outcome runs the other block
 * as long as the result of the first one equals boolean False.
 */
ctr_object* ctr_block_while_false(ctr_object* myself, ctr_argument* argumentList) {
	while (1) {
		ctr_object* result = ctr_internal_cast2bool(ctr_block_run(myself, argumentList, myself));
		if (result->value.bvalue == 1) break;
		ctr_block_run(argumentList->object, argumentList, argumentList->object);
	}
	return myself;
}

/**
 * Alias for BlockRun.
 */
ctr_object* ctr_block_runIt(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_block_run(myself, argumentList, myself);
}

/**
 * BlockError
 *
 * Sets error flag on a block of code.
 */
ctr_object* ctr_block_error(ctr_object* myself, ctr_argument* argumentList) {
	error = argumentList->object;
	return myself;
}

/**
 * BlockCatch
 *
 * Associates an error clause to a block.
 * If an error (exception) occurs within the block this block will be
 * executed.
 */
ctr_object* ctr_block_catch(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* catchBlock = argumentList->object;
	ctr_internal_object_delete_property(myself, ctr_build_string("catch",5),0);
	ctr_internal_object_add_property(myself, ctr_build_string("catch",5), catchBlock, 0);
	return myself;
}

/**
 * Builds a block object from a literal block of code.
 */
ctr_object* ctr_build_block(tnode* node) {
	ctr_object* codeBlockObject = ctr_internal_create_object(OTBLOCK);
	codeBlockObject->value.block = node;
	codeBlockObject->link = CBlock;
	return codeBlockObject;
}
