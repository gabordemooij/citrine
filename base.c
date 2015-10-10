
/**
 * Builds a Nil object.
 *
 * Literal form: Nil
 */
obj* ctr_build_nil() {	
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
obj* ctr_object_make() {
	obj* objectInstance = NULL;
	objectInstance = ctr_internal_create_object(OTOBJECT);
	objectInstance->link = Object;
	return objectInstance;
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
obj* ctr_object_equals(obj* myself, args* argumentList) {
	obj* otherObject = argumentList->object;
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
obj* ctr_object_on_do(obj* myself, args* argumentList) {
	obj* methodName = argumentList->object;
	if (methodName->info.type != OTSTRING) {
		error = ctr_build_string_from_cstring("Expected on: argument to be of type string.");
		return myself;
	}
	args* nextArgument = argumentList->next;
	obj* methodBlock = nextArgument->object;
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
obj* ctr_object_override_does(obj* myself, args* argumentList) {
	obj* methodName = argumentList->object;
	if (methodName->info.type != OTSTRING) {
		error = ctr_build_string_from_cstring("Expected on: argument to be of type string.");
		return myself;
	}
	args* nextArgument = argumentList->next;
	obj* methodBlock = nextArgument->object;
	if (methodBlock->info.type != OTBLOCK) {
		error = ctr_build_string_from_cstring("Expected argument do: to be of type block.");
		return myself;
	}
	obj* overriddenMethod = ctr_internal_object_find_property(myself, methodName, 1);
	if (overriddenMethod == NULL) {
		error = ctr_build_string_from_cstring("Cannot override, original response not found.");
		return myself;
	}
	char* superMethodNameString = malloc(sizeof(char) * (methodName->value.svalue->vlen + 11));
	memcpy(superMethodNameString, "overridden-", (sizeof(char) * 11));
	memcpy(superMethodNameString + (sizeof(char)*11), methodName->value.svalue->value, methodName->value.svalue->vlen);
	obj* superMethodKey = ctr_build_string(superMethodNameString, (methodName->value.svalue->vlen + 11));
	ctr_internal_object_delete_property(myself, methodName, 1);
	ctr_internal_object_add_property(myself, superMethodKey, overriddenMethod, 1);
	ctr_internal_object_add_property(myself, methodName, methodBlock, 1);
	return myself;
}


/**
 * Default respond-to implemention, does nothing.
 */
obj* ctr_object_respond(obj* myself, args* argumentList) {
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
obj* ctr_object_basedOn(obj* myself, args* argumentList) {
	obj* other = argumentList->object;
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
obj* ctr_build_bool(int truth) {
	obj* boolObject = ctr_internal_create_object(OTBOOL);
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
obj* ctr_bool_iftrue(obj* myself, args* argumentList) {
	if (myself->value.bvalue) {
		obj* codeBlock = argumentList->object;
		args* arguments = CTR_CREATE_ARGUMENT();
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
obj* ctr_bool_ifFalse(obj* myself, args* argumentList) {
	if (!myself->value.bvalue) {
		obj* codeBlock = argumentList->object;
		args* arguments = CTR_CREATE_ARGUMENT();
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
obj* ctr_bool_opposite(obj* myself, args* argumentList) {
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
obj* ctr_bool_and(obj* myself, args* argumentList) {
	obj* other = ctr_internal_cast2bool(argumentList->object);
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
obj* ctr_bool_or(obj* myself, args* argumentList) {
	obj* other = ctr_internal_cast2bool(argumentList->object);	
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
obj* ctr_bool_xor(obj* myself, args* argumentList) {
	obj* other = ctr_internal_cast2bool(argumentList->object);	
	return ctr_build_bool((myself->value.bvalue ^ other->value.bvalue));
}


/**
 * BuildNumber
 *
 * Creates a number from a C string (0 terminated).
 * Internal use only.
 */
obj* ctr_build_number(char* n) {
	obj* numberObject = ctr_internal_create_object(OTNUMBER);
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
obj* ctr_build_number_from_float(float f) {
	obj* numberObject = ctr_internal_create_object(OTNUMBER);
	numberObject->value.nvalue = f;
	numberObject->link = Number;
	return numberObject;
}

/**
 * Number Comparison Operators
 *
 * Implementation of >, <, >=, <=, == and !=
 */
obj* ctr_number_higherThan(obj* myself, args* argumentList) {
	obj* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue > otherNum->value.nvalue));
}

obj* ctr_number_higherEqThan(obj* myself, args* argumentList) {
	obj* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue >= otherNum->value.nvalue));
}

obj* ctr_number_lowerThan(obj* myself, args* argumentList) {
	obj* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue < otherNum->value.nvalue));
}

obj* ctr_number_lowerEqThan(obj* myself, args* argumentList) {
	obj* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue <= otherNum->value.nvalue));
}

obj* ctr_number_eq(obj* myself, args* argumentList) {
	obj* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool(myself->value.nvalue == otherNum->value.nvalue);
}

obj* ctr_number_neq(obj* myself, args* argumentList) {
	obj* otherNum = ctr_internal_cast2number(argumentList->object);
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
obj* ctr_number_between(obj* myself, args* argumentList) {
	obj* otherNum = ctr_internal_cast2number(argumentList->object);
	args* nextArgumentItem = argumentList->next;
	obj* nextArgument = ctr_internal_cast2number(nextArgumentItem->object);
	return ctr_build_bool((myself->value.nvalue >=  otherNum->value.nvalue) && (myself->value.nvalue <= nextArgument->value.nvalue));
}


/**
 * Arithmetic
 * 
 * The following functions implement basic arithmetic operations:
 * 
 * + - / * inc dec mul div
 */
obj* ctr_string_concat(obj* myself, args* argumentList);
obj* ctr_number_add(obj* myself, args* argumentList) {
	obj* otherNum = argumentList->object;
	if (otherNum->info.type == OTSTRING) {
		obj* strObject = ctr_internal_create_object(OTSTRING);
		strObject = ctr_internal_cast2string(myself);
		args* newArg = CTR_CREATE_ARGUMENT();
		newArg->object = otherNum;
		return ctr_string_concat(strObject, newArg);
	}
	float a = myself->value.nvalue;
	float b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a+b));
}


obj* ctr_number_inc(obj* myself, args* argumentList) {
	obj* otherNum = ctr_internal_cast2number(argumentList->object);
	myself->value.nvalue += otherNum->value.nvalue;
	return myself;
}

obj* ctr_number_minus(obj* myself, args* argumentList) {
	obj* otherNum = ctr_internal_cast2number(argumentList->object);
	float a = myself->value.nvalue;
	float b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a-b));
}

obj* ctr_number_dec(obj* myself, args* argumentList) {
	obj* otherNum = ctr_internal_cast2number(argumentList->object);
	myself->value.nvalue -= otherNum->value.nvalue;
	return myself;
}

obj* ctr_number_multiply(obj* myself, args* argumentList) {
	obj* otherNum = ctr_internal_cast2number(argumentList->object);
	float a = myself->value.nvalue;
	float b = otherNum->value.nvalue;
	return ctr_build_number_from_float(a*b);
}

obj* ctr_number_mul(obj* myself, args* argumentList) {
	obj* otherNum = ctr_internal_cast2number(argumentList->object);
	myself->value.nvalue *= otherNum->value.nvalue;
	return myself;
}

obj* ctr_number_divide(obj* myself, args* argumentList) {
	obj* otherNum = ctr_internal_cast2number(argumentList->object);
	float a = myself->value.nvalue;
	float b = otherNum->value.nvalue;
	if (b == 0) {
		error = ctr_build_string_from_cstring("Division by zero.");
		return myself;
	}
	return ctr_build_number_from_float((a/b));
}

obj* ctr_number_div(obj* myself, args* argumentList) {
	obj* otherNum = ctr_internal_cast2number(argumentList->object);
	if (otherNum->value.nvalue == 0) {
		error = ctr_build_string_from_cstring("Division by zero.");
		return myself;
	}
	myself->value.nvalue /= otherNum->value.nvalue;
	return myself;
}

obj* ctr_number_modulo(obj* myself, args* argumentList) {
	obj* otherNum = ctr_internal_cast2number(argumentList->object);
	float a = myself->value.nvalue;
	float b = otherNum->value.nvalue;
	if (b == 0) {
		error = ctr_build_string_from_cstring("Division by zero.");
		return myself;
	}
	return ctr_build_number_from_float(fmod(a,b));
}

obj* ctr_number_pow(obj* myself, args* argumentList) {
	obj* otherNum = ctr_internal_cast2number(argumentList->object);
	float a = myself->value.nvalue;
	float b = otherNum->value.nvalue;
	return ctr_build_number_from_float(pow(a,b));
}

obj* ctr_number_max(obj* myself, args* argumentList) {
	obj* otherNum = ctr_internal_cast2number(argumentList->object);
	float a = myself->value.nvalue;
	float b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a >= b) ? a : b);
}

obj* ctr_number_min(obj* myself, args* argumentList) {
	obj* otherNum = ctr_internal_cast2number(argumentList->object);
	float a = myself->value.nvalue;
	float b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a <= b) ? a : b);
}

/**
 * Factorial
 *
 * Calculates the factorial of a number.
 */
obj* ctr_number_factorial(obj* myself, args* argumentList) {
	float t = myself->value.nvalue;
	int i;
	float a = 1;
	for(i = (int) t; i > 0; i--) {
		a = a * i;
	}
	return ctr_build_number_from_float(a);
}

/**
 * Some basic math functions: floor, ceil, round, abs, sqrt, pow, sin, cos
 * tan, atan and log.
 */
obj* ctr_number_floor(obj* myself, args* argumentList) {
	return ctr_build_number_from_float(floor(myself->value.nvalue));
}

obj* ctr_number_ceil(obj* myself, args* argumentList) {
	return ctr_build_number_from_float(ceil(myself->value.nvalue));
}

obj* ctr_number_round(obj* myself, args* argumentList) {
	return ctr_build_number_from_float(round(myself->value.nvalue));
}

obj* ctr_number_abs(obj* myself, args* argumentList) {
	return ctr_build_number_from_float(fabs(myself->value.nvalue));
}

obj* ctr_number_sqrt(obj* myself, args* argumentList) {
	return ctr_build_number_from_float(sqrt(myself->value.nvalue));
}

obj* ctr_number_exp(obj* myself, args* argumentList) {
	return ctr_build_number_from_float(exp(myself->value.nvalue));
}

obj* ctr_number_sin(obj* myself, args* argumentList) {
	return ctr_build_number_from_float(sin(myself->value.nvalue));
}

obj* ctr_number_cos(obj* myself, args* argumentList) {
	return ctr_build_number_from_float(cos(myself->value.nvalue));
}

obj* ctr_number_tan(obj* myself, args* argumentList) {
	return ctr_build_number_from_float(tan(myself->value.nvalue));
}

obj* ctr_number_atan(obj* myself, args* argumentList) {
	return ctr_build_number_from_float(atan(myself->value.nvalue));
}

obj* ctr_number_log(obj* myself, args* argumentList) {
	return ctr_build_number_from_float(log(myself->value.nvalue));
}


/**
 * Times
 *
 * Runs the specified code block N times.
 *
 * Usage:
 * 7 times: { ... }.
 */
obj* ctr_number_times(obj* myself, args* argumentList) {
	obj* block = argumentList->object;
	if (block->info.type != OTBLOCK) { printf("Expected code block."); exit(1); }
	block->info.sticky = 1; //mark as sticky
	int t = myself->value.nvalue;
	int i;
	for(i=0; i<t; i++) {
		char* nstr = (char*) calloc(20, sizeof(char));
		snprintf(nstr, 20, "%d", i);
		obj* indexNumber = ctr_build_number(nstr);
		args* arguments = CTR_CREATE_ARGUMENT();
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
obj* ctr_build_string(char* stringValue, long size) {
	obj* stringObject = ctr_internal_create_object(OTSTRING);
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
obj* ctr_build_string_from_cstring(char* cstring) {
	return ctr_build_string(cstring, strlen(cstring));
}

/**
 * StringBytes
 *
 * Returns the number of bytes in a string.
 */
obj* ctr_string_bytes(obj* myself, args* argumentList) {
	return ctr_build_number_from_float((float)myself->value.svalue->vlen);
}

/**
 * StringEquality
 *
 * Returns True if the other string is the same (in bytes).
 */
obj* ctr_string_eq(obj* myself, args* argumentList) {
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
obj* ctr_string_neq(obj* myself, args* argumentList) {
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
obj* ctr_string_length(obj* myself, args* argumentList) {
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
obj* ctr_string_concat(obj* myself, args* argumentList) {
	obj* strObject = ctr_internal_create_object(OTSTRING);
	strObject = ctr_internal_cast2string(argumentList->object);
	long n1 = myself->value.svalue->vlen;
	long n2 = strObject->value.svalue->vlen;
	char* dest = calloc(sizeof(char), (n1 + n2));
	memcpy(dest, myself->value.svalue->value, n1);
	memcpy(dest+n1, strObject->value.svalue->value, n2);
	obj* newString = ctr_build_string(dest, (n1 + n2));
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
obj* ctr_string_fromto(obj* myself, args* argumentList) {
	obj* fromPos = ctr_internal_cast2number(argumentList->object);
	obj* toPos = ctr_internal_cast2number(argumentList->next->object);
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
	obj* newString = ctr_build_string(dest,ub);
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
obj* ctr_string_from_length(obj* myself, args* argumentList) {
	obj* fromPos = ctr_internal_cast2number(argumentList->object);
	obj* length = ctr_internal_cast2number(argumentList->next->object);
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
	obj* newString = ctr_build_string(dest,ub);
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
obj* ctr_string_at(obj* myself, args* argumentList) {
	obj* fromPos = ctr_internal_cast2number(argumentList->object);
	long a = (fromPos->value.nvalue);
	long ua = getBytesUtf8(myself->value.svalue->value, 0, a);
	long ub = getBytesUtf8(myself->value.svalue->value, ua, 1);
	char* dest = malloc(ub * sizeof(char));
	memcpy(dest, (myself->value.svalue->value) + ua, ub);
	obj* newString = ctr_build_string(dest,ub);
	return newString;
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
obj* ctr_string_index_of(obj* myself, args* argumentList) {
	obj* sub = ctr_internal_cast2string(argumentList->object);
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
obj* ctr_string_last_index_of(obj* myself, args* argumentList) {
	obj* sub = ctr_internal_cast2string(argumentList->object);
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
obj* ctr_string_replace_with(obj* myself, args* argumentList) {
	obj* needle = ctr_internal_cast2string(argumentList->object);
	obj* replacement = ctr_internal_cast2string(argumentList->next->object);
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
obj* ctr_string_trim(obj* myself, args* argumentList) {
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

obj* ctr_block_run(obj* myself, args* argList, obj* my) {
	obj* result;
	tnode* node = myself->value.block;
	tlistitem* codeBlockParts = node->nodes;
	tnode* codeBlockPart1 = codeBlockParts->node;
	tnode* codeBlockPart2 = codeBlockParts->next->node;
	tlistitem* parameterList = codeBlockPart1->nodes;
	tnode* parameter;
	ctr_open_context();
	if (parameterList && parameterList->node) {
		parameter = parameterList->node;
		obj* a;
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
		obj* catchBlock = malloc(sizeof(obj));
		catchBlock = ctr_internal_object_find_property(myself, ctr_build_string("catch",5), 0);
		if (catchBlock != NULL) {
			args* a = CTR_CREATE_ARGUMENT();
			a->object = error;
			error = NULL;
			ctr_block_run(catchBlock, a, my);
			result = myself;
		}
	}
	return result;
}

obj* ctr_block_runIt(obj* myself, args* argumentList) {
	return ctr_block_run(myself, argumentList, myself);
}

obj* ctr_block_error(obj* myself, args* argumentList) {
	error = argumentList->object;
	return myself;
}

obj* ctr_block_catch(obj* myself, args* argumentList) {
	obj* catchBlock = argumentList->object;
	ctr_internal_object_delete_property(myself, ctr_build_string("catch",5),0);
	ctr_internal_object_add_property(myself, ctr_build_string("catch",5), catchBlock, 0);
	return myself;
}

obj* ctr_build_block(tnode* node) {
	obj* codeBlockObject = ctr_internal_create_object(OTBLOCK);
	codeBlockObject->value.block = node;
	codeBlockObject->link = CBlock;
	return codeBlockObject;
}


