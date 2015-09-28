
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

//create number from \0 terminated string
obj* ctr_build_number(char* n) {
	obj* numberObject = ctr_internal_create_object(OTNUMBER);
	numberObject->value.nvalue = atof(n);
	numberObject->link = Number;
	return numberObject;
}

obj* ctr_build_number_from_float(float f) {
	obj* numberObject = ctr_internal_create_object(OTNUMBER);
	numberObject->value.nvalue = f;
	numberObject->link = Number;
	return numberObject;
}

obj* ctr_number_higherThan(obj* myself, args* argumentList) {
	obj* otherNum = argumentList->object;
	if (otherNum->info.type != OTNUMBER) { printf("Expected number."); exit(1); }
	return ctr_build_bool((myself->value.nvalue > otherNum->value.nvalue));
}

obj* ctr_number_higherEqThan(obj* myself, args* argumentList) {
	obj* otherNum = argumentList->object;
	if (otherNum->info.type != OTNUMBER) { printf("Expected number."); exit(1); }
	return ctr_build_bool((myself->value.nvalue >= otherNum->value.nvalue));
}

obj* ctr_number_lowerThan(obj* myself, args* argumentList) {
	obj* otherNum = argumentList->object;
	if (otherNum->info.type != OTNUMBER) { printf("Expected number."); exit(1); }
	return ctr_build_bool((myself->value.nvalue < otherNum->value.nvalue));
}

obj* ctr_number_lowerEqThan(obj* myself, args* argumentList) {
	obj* otherNum = argumentList->object;
	if (otherNum->info.type != OTNUMBER) { printf("Expected number."); exit(1); }
	return ctr_build_bool((myself->value.nvalue <= otherNum->value.nvalue));
}

obj* ctr_number_eq(obj* myself, args* argumentList) {
	obj* otherNum = argumentList->object;
	if (otherNum->info.type != OTNUMBER) { printf("Expected number."); exit(1); }
	return ctr_build_bool(myself->value.nvalue == otherNum->value.nvalue);
}

obj* ctr_number_neq(obj* myself, args* argumentList) {
	obj* otherNum = argumentList->object;
	if (otherNum->info.type != OTNUMBER) { printf("Expected number."); exit(1); }
	return ctr_build_bool(myself->value.nvalue != otherNum->value.nvalue);
}

obj* ctr_number_between(obj* myself, args* argumentList) {
	obj* otherNum = argumentList->object;
	if (otherNum->info.type != OTNUMBER) { printf("Expected number."); exit(1); }
	if (!argumentList->next) { printf("Expected second number."); exit(1); }
	args* nextArgumentItem = argumentList->next;
	obj* nextArgument = nextArgumentItem->object;
	if (nextArgument->info.type != OTNUMBER) { printf("Expected second argument to be number."); exit(1); }
	return ctr_build_bool((myself->value.nvalue >=  otherNum->value.nvalue) && (myself->value.nvalue <= nextArgument->value.nvalue));
}

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
	else if (otherNum->info.type != OTNUMBER) { printf("Expected number."); exit(1); }
	float a = myself->value.nvalue;
	float b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a+b));
}


obj* ctr_number_inc(obj* myself, args* argumentList) {
	obj* otherNum = argumentList->object;
	if (otherNum->info.type != OTNUMBER) { printf("Expected number."); exit(1); }
	myself->value.nvalue += otherNum->value.nvalue;
	return myself;
}

obj* ctr_number_minus(obj* myself, args* argumentList) {
	obj* otherNum = argumentList->object;
	if (otherNum->info.type != OTNUMBER) { printf("Expected number."); exit(1); }
	float a = myself->value.nvalue;
	float b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a-b));
}

obj* ctr_number_dec(obj* myself, args* argumentList) {
	obj* otherNum = argumentList->object;
	if (otherNum->info.type != OTNUMBER) { printf("Expected number."); exit(1); }
	myself->value.nvalue -= otherNum->value.nvalue;
	return myself;
}

obj* ctr_number_multiply(obj* myself, args* argumentList) {
	obj* otherNum = argumentList->object;
	if (otherNum->info.type != OTNUMBER) { printf("Expected number."); exit(1); }
	float a = myself->value.nvalue;
	float b = otherNum->value.nvalue;
	return ctr_build_number_from_float(a*b);
}

obj* ctr_number_mul(obj* myself, args* argumentList) {
	obj* otherNum = argumentList->object;
	if (otherNum->info.type != OTNUMBER) { printf("Expected number."); exit(1); }
	myself->value.nvalue *= otherNum->value.nvalue;
	return myself;
}


obj* ctr_number_divide(obj* myself, args* argumentList) {
	obj* otherNum = argumentList->object;
	if (otherNum->info.type != OTNUMBER) { printf("Expected number."); exit(1); }
	float a = myself->value.nvalue;
	float b = otherNum->value.nvalue;
	if (b == 0) {
		printf("Division by zero.");
		exit(1);
	}
	return ctr_build_number_from_float((a/b));
}

obj* ctr_number_div(obj* myself, args* argumentList) {
	obj* otherNum = argumentList->object;
	if (otherNum->info.type != OTNUMBER) { printf("Expected number."); exit(1); }
	if (otherNum->value.nvalue == 0) {
		printf("Division by zero.");
		exit(1);
	}
	myself->value.nvalue /= otherNum->value.nvalue;
	return myself;
}


obj* ctr_number_factorial(obj* myself, args* argumentList) {
	float t = myself->value.nvalue;
	int i;
	float a = 1;
	for(i = (int) t; i > 0; i--) {
		a = a * i;
	}
	myself->value.nvalue = a;
	return myself;
}

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




obj* ctr_build_string(char* stringValue, long size) {
	obj* stringObject = ctr_internal_create_object(OTSTRING);
	ASSIGN_STRING(stringObject->value.svalue, value, stringValue, size);
	stringObject->value.svalue->vlen = size;
	stringObject->link = TextString;
	return stringObject;
}

obj* ctr_build_string_from_cstring(char* cstring) {
	return ctr_build_string(cstring, strlen(cstring));
}

obj* ctr_string_bytes(obj* myself, args* argumentList) {
	char* str = calloc(100, sizeof(char));
	long l = (myself->value.svalue->vlen);
	sprintf(str, "%lu", l);
	return ctr_build_number(str);
}

obj* ctr_string_eq(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	if (argumentList->object->value.svalue->vlen != myself->value.svalue->vlen) {
		return ctr_build_bool(0);
	}
	return ctr_build_bool((strncmp(argumentList->object->value.svalue->value, myself->value.svalue->value, myself->value.svalue->vlen)==0));
}

obj* ctr_string_length(obj* myself, args* argumentList) {
	long n = getutf8len(myself->value.svalue->value, myself->value.svalue->vlen);
	char* str = calloc(100, sizeof(char));
	sprintf(str, "%lu", n);
	return ctr_build_number(str);
}

obj* ctr_string_printbytes(obj* myself, args* argumentList) {
	char* str = myself->value.svalue->value;
	long n = myself->value.svalue->vlen;
	long i = 0;
	for(i = 0; i < n; i++) printf("%u ", (unsigned char) str[i]);
	printf("\n");
	return myself;
}

obj* ctr_string_concat(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
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

obj* ctr_string_fromto(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	if (!argumentList->next) {
		printf("Missing argument 2\n"); exit(1);
	}
	obj* fromPos = argumentList->object;
	obj* toPos = argumentList->next->object;
	
	long a = (fromPos->value.nvalue);
	long b = (toPos->value.nvalue); 
	long ua = getBytesUtf8(myself->value.svalue->value, 0, a);
	long ub = getBytesUtf8(myself->value.svalue->value, ua, ((b - a) + 1));
	char* dest = calloc(ub, sizeof(char));
	memcpy(dest, (myself->value.svalue->value) + ua, ub);
	obj* newString = ctr_build_string(dest,ub);
	return newString;
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


