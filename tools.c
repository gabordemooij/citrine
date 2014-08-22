#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>

#include "uthash.h"
#include "object.h"
#include "parser.h"
#include "walker.h"

#define OTNIL 80
#define OTBOOL 81
#define OTNUMBER 82
#define OTSTRING 83
#define OTBLOCK 84
#define OTOBJECT 85
#define OTNATFUNC 86
#define OTCUST 87
#define OTMISC 88
#define OTEX 89

obj* World = NULL;
obj* contexts[100];
int cid = 0;
obj* dnk_build_number(char* number);
obj* Object;
obj* Number;
obj* BoolX;
obj* Nil;
int debug;

char* readf(char* file_name) {
   char* prg;
   char ch;
   FILE* fp;
   fp = fopen(file_name,"r");
   if( fp == NULL )
   {
      printf("Error while opening the file.\n");
      exit(1);
   }
   int prev = ftell(fp);
   fseek(fp,0L,SEEK_END);
   int sz = ftell(fp);
   fseek(fp,prev,SEEK_SET);
   prg = malloc((sz+1)*sizeof(char));
   int i=0;
   while( ( ch = fgetc(fp) ) != EOF )
     prg[i++]=ch;
   fclose(fp);
   return prg;
}

void tree(tnode* ti, int indent) {
	int antiCrash = 0;
	if (indent>20) exit(1); 
	tlistitem* li = ti->nodes;
	tnode* t = li->node;
	while((antiCrash++<100)) {
		int i = 0;
		for (i=0; i<indent; i++) printf(" ");
		char* str = calloc(40, sizeof(char));
		if (t->type == EXPRASSIGNMENT) 		str = "ASSIGN\0";
		else if (t->type == EXPRMESSAGE) 	str = "MESSAG\0";
		else if (t->type == UNAMESSAGE) 	str = "UMSSAG\0";
		else if (t->type == KWMESSAGE) 		str = "KMSSAG\0";
		else if (t->type == BINMESSAGE) 	str = "BMSSAG\0";
		else if (t->type == LTRSTRING) 		str = "STRING\0";
		else if (t->type == REFERENCE) 		str = "REFRNC\0";
		else if (t->type == LTRNUM) 		str = "NUMBER\0";
		else if (t->type == CODEBLOCK) 		str = "CODEBL\0";
		else if (t->type == RETURNFROMBLOCK)str = "RETURN\0";
		else if (t->type == PARAMLIST)		str = "PARAMS\0";
		else if (t->type == INSTRLIST)		str = "INSTRS\0";
		else if (t->type == ENDOFPROGRAM)	str = "EOPROG\0";
		else if (t->type == NESTED)	        str = "NESTED\0";
		else if (t->type == LTRBOOLFALSE)	str = "BFALSE\0";
		else if (t->type == LTRBOOLTRUE)	str = "BLTRUE\0";
		else if (t->type == LTRNIL)	        str = "LTRNIL\0";
		else 								str = "UNKNW?\0";
		
		printf("%d:%s %s\n", t->type, str, t->value);
		if (t->nodes) tree(t, indent + 1);
		if (!li->next) break; 
		li = li->next;
		t = li->node;
	}
}

void dnk_open_context() {
	cid++;
	obj* context = NULL;
	context = malloc(sizeof(obj*));
	context->name = "Context";
	context->value = NULL;
	contexts[cid] = context;
}

void dnk_close_context() {
	if (cid == 0) return;
	cid--;
}

obj* dnk_find(char* key) {
	int i = cid;
	obj* foundObject = NULL;
	foundObject = calloc(sizeof(obj*), 1);
	int antiCrash = 0;
	int first = 1;
	while(((antiCrash++ < 100) && i>-1 && foundObject == NULL) || first) {
		first = 0;
		obj* context = contexts[i];
		HASH_FIND_STR(context->properties, key, foundObject);
		i--;
	}
	if (foundObject == NULL) { printf("Error, key not found: %s.\n", key); exit(1); }
	return foundObject;
}

obj* dnk_find_in_my(char* key) {
	obj* foundObject = O();
	obj* context = dnk_find("me");
	HASH_FIND_STR(context->link->properties, key, foundObject);
	if (foundObject == NULL) { printf("Error, property not found: %s.\n", key); exit(1); }
	return foundObject;
}


void dnk_set(obj* object) {
	obj* context = contexts[cid];
	HASH_ADD_KEYPTR(hh, context->properties, object->name, strlen(object->name), object);
}

obj* dnk_build_bool(int truth) {
	obj* boolObject = O();
	boolObject->name = "Bool";
	if (truth) boolObject->value = "1"; else boolObject->value = "0";
	boolObject->type = OTBOOL;
	boolObject->link = BoolX;
	return boolObject;
}


obj* dnk_pencil_write(obj* myself, args* argumentList) {
	obj* argument1 = argumentList->object;
	printf("----------------> OUTPUT: %s\n", argument1->value);
	return myself;
}

obj* dnk_block_run(obj* myself, args* argList, obj* my) {
	obj* selfRef = O();
	selfRef->name = "me";
	selfRef->type = OTOBJECT;
	selfRef->value = "[self]";
	selfRef->link = my;
	obj* result;
	tnode* node = myself->block;
	tlistitem* codeBlockParts = node->nodes;
	tnode* codeBlockPart1 = codeBlockParts->node;
	tnode* codeBlockPart2 = codeBlockParts->next->node;
	tlistitem* parameterList = codeBlockPart1->nodes;
	tnode* parameter;
	if (parameterList && parameterList->node) {
		parameter = parameterList->node;
		int antiCrash = 0;
		obj* a;
		while((antiCrash++ < 100)) {
			if (parameter && argList->object) {
				a = argList->object;
				a->name = parameter->value;
				dnk_set(a);
			}
			if (!argList->next) break;
			argList = argList->next;
			if (!parameterList->next) break;
			parameterList = parameterList->next;
			parameter = parameterList->node;
		}
	}
	
	dnk_open_context();
	dnk_set(selfRef);
	result = dwlk_run(codeBlockPart2);
	dnk_close_context();
	return result;
}

obj* dnk_build_block(tnode* node) {
	obj* codeBlockObject = O();
	codeBlockObject->type = OTBLOCK;
	codeBlockObject->block = node;
	codeBlockObject->value = "[block]";
	return codeBlockObject;
}

obj* dnk_bool_iftrue(obj* myself, args* argumentList) {
	if (strncmp(myself->value,"1",1)==0) {
		obj* codeBlock = argumentList->object;
		args* arguments = A();
		arguments->object = myself;
		return dnk_block_run(codeBlock, arguments, myself);
	}
	return myself;
}

obj* dnk_bool_ifFalse(obj* myself, args* argumentList) {
	if (strncmp(myself->value,"0",1)==0) {
		obj* codeBlock = argumentList->object;
		args* arguments = A();
		arguments->object = myself;
		return dnk_block_run(codeBlock, arguments, myself);
	}
	return myself;
}

obj* dnk_number_higherThan(obj* myself, args* argumentList) {
	obj* otherNum = argumentList->object;
	if (otherNum->type != OTNUMBER) { printf("Expected number."); exit(1); }
	float a = atof(myself->value);
	float b = atof(otherNum->value);
	obj* truth = dnk_build_bool((a > b));
	return truth;
}

obj* dnk_number_add(obj* myself, args* argumentList) {
	obj* otherNum = argumentList->object;
	if (otherNum->type != OTNUMBER) { printf("Expected number."); exit(1); }
	float a = atof(myself->value);
	float b = atof(otherNum->value);
	char* str = calloc(sizeof(char), 40);
	sprintf(str, "%f", (a+b));
	myself->value = str;
	return myself;
}

obj* dnk_number_minus(obj* myself, args* argumentList) {
	obj* otherNum = argumentList->object;
	if (otherNum->type != OTNUMBER) { printf("Expected number."); exit(1); }
	float a = atof(myself->value);
	float b = atof(otherNum->value);
	char* str = calloc(sizeof(char), 40);
	sprintf(str, "%f", (a-b));
	myself->value = str;
	return myself;
}

obj* dnk_number_factorial(obj* myself, args* argumentList) {
	float t = floor(atof(myself->value));
	int i;
	float a = 1;
	for(i = (int) t; i > 0; i--) {
		a = a * i;
	}
	char* str = calloc(sizeof(char), 40);
	sprintf(str, "%f", (a));
	myself->value = str;
	return myself;
}

obj* dnk_number_times(obj* myself, args* argumentList) {
	obj* block = argumentList->object;
	if (block->type != OTBLOCK) { printf("Expected code block."); exit(1); }
	int t = atoi(myself->value);
	int i;
	for(i=0; i<t; i++) {
		char* nstr = (char*) calloc(20, sizeof(char));
		snprintf(nstr, 20, "%d", i);
		obj* indexNumber = dnk_build_number(nstr);
		args* arguments = A();
		arguments->object = indexNumber;
		dnk_block_run(block, arguments, myself);
	}
	return myself;
}

obj* dnk_build_number(char* n) {
	obj* numberObject = O();
	numberObject->name = "Number";
	numberObject->value = malloc(sizeof(char)*strlen(n));
	strcpy(numberObject->value, n);
	numberObject->type = OTNUMBER;
	numberObject->link = Number;
	return numberObject;
}

obj* dnk_object_make() {
	obj* objectInstance = NULL;
	objectInstance = O();
	objectInstance->type = OTOBJECT;
	objectInstance->value = "[object]";
	objectInstance->link = Object;
	return objectInstance;
}

obj* dnk_object_method_does(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	if (!argumentList->next) {
		printf("Missing argument 2\n"); exit(1);
	}
	obj* methodName = argumentList->object;
	if (methodName->type != OTSTRING) {
		printf("Expected argument method: to be of type string.\n");
		exit(1);
	}
	args* nextArgument = argumentList->next;
	if (!nextArgument->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	obj* methodBlock = nextArgument->object;
	if (methodBlock->type != OTBLOCK) {
		printf("Expected argument does: to be of type block.\n");
		exit(1);
	}
	methodBlock->name = methodName->value;
	if (debug) printf("Adding method block: %s %s to %s.\n", methodBlock->value, methodBlock->name, myself->name);
	HASH_ADD_KEYPTR(hh, myself->methods, methodBlock->name, strlen(methodBlock->name), methodBlock);
	return myself;
}


obj* dnk_object_override_does(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	if (!argumentList->next) {
		printf("Missing argument 2\n"); exit(1);
	}
	obj* methodName = argumentList->object;
	if (methodName->type != OTSTRING) {
		printf("Expected argument method: to be of type string.\n");
		exit(1);
	}
	args* nextArgument = argumentList->next;
	if (!nextArgument->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	obj* methodBlock = nextArgument->object;
	if (methodBlock->type != OTBLOCK) {
		printf("Expected argument does: to be of type block.\n");
		exit(1);
	}
	methodBlock->name = methodName->value;
	if (debug) printf("Adding method block: %s %s to %s.\n", methodBlock->value, methodBlock->name, myself->name);
	obj* oldBlock = O();
	HASH_FIND_STR(myself->methods, methodBlock->name, oldBlock);
	if (!oldBlock) printf("Cannot override: %s no such method.", oldBlock->name);
	char* str = (char*) calloc(sizeof(char),255);
	strcat(str, "overridden-");
	strcat(str, oldBlock->name);
	oldBlock->name = str;
	HASH_DEL(myself->methods, oldBlock);
	HASH_ADD_KEYPTR(hh, myself->methods, oldBlock->name, strlen(oldBlock->name), oldBlock);
	HASH_ADD_KEYPTR(hh, myself->methods, methodBlock->name, strlen(methodBlock->name), methodBlock);
	return myself;
}


obj* dnk_object_blueprint(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	obj* other = argumentList->object;
	if (other->type != OTOBJECT) {
		printf("Expected argument method: to be of type object.\n");
		exit(1);
	}
	myself->link = other;
	return myself;
}

obj* dnk_build_string(char* stringValue) {
	obj* stringObject = O();
	stringObject->type = OTSTRING;
	stringObject->value = calloc(sizeof(char), strlen(stringValue));
	strcpy(stringObject->value, stringValue);
	return stringObject;
}

obj* dnk_build_nil() {	
	return Nil;
}

obj* dnk_nil_isnil(obj* myself, args* argumentList) {
	obj* truth;
	if ((myself->type == OTNIL)) {
		truth = dnk_build_bool(1);
	} else {
		truth = dnk_build_bool(0);
	}
	return truth;
}

void dnk_initialize_world() {
	obj* Pencil = NULL;
	obj* PencilWrite = NULL;
	World = (obj*) malloc(sizeof(obj));
	contexts[0] = World;
	Pencil = (obj*) malloc(sizeof(obj));
	PencilWrite = (obj*) malloc(sizeof(obj));
	World->name = "World";
	Pencil->name = "Pencil";
	PencilWrite->name = "write:";
	PencilWrite->type = OTNATFUNC;
	PencilWrite->value = (void*) &dnk_pencil_write;
	HASH_ADD_KEYPTR(hh, World->properties, Pencil->name, strlen(Pencil->name), Pencil);
	HASH_ADD_KEYPTR(hh, Pencil->methods, PencilWrite->name, strlen(PencilWrite->name), PencilWrite);
	
	Object = O();
	obj* ObjectMake = O();
	obj* ObjectMethodDoes = O();
	Object->name = "Object";
	Object->type = OTOBJECT;
	Object->value = "[object]";
	ObjectMake->name = "new";
	ObjectMake->type = OTNATFUNC;
	ObjectMake->value = (void*) &dnk_object_make;
	HASH_ADD_KEYPTR(hh, World->properties, Object->name, strlen(Object->name), Object);
	HASH_ADD_KEYPTR(hh, Object->methods, ObjectMake->name, strlen(ObjectMake->name), ObjectMake);
	
	ObjectMethodDoes->name = "method:does:";
	ObjectMethodDoes->type = OTNATFUNC;
	ObjectMethodDoes->value = (void*) &dnk_object_method_does;
	HASH_ADD_KEYPTR(hh, Object->methods, ObjectMethodDoes->name, strlen(ObjectMethodDoes->name), ObjectMethodDoes);
	
	obj* ObjectOverrideDoes = O();
	ObjectOverrideDoes->name = "override:does:";
	ObjectOverrideDoes->type = OTNATFUNC;
	ObjectOverrideDoes->value = (void*) &dnk_object_override_does;
	HASH_ADD_KEYPTR(hh, Object->methods, ObjectOverrideDoes->name, strlen(ObjectOverrideDoes->name), ObjectOverrideDoes);
	
	obj* ObjectBlueprint = O();
	ObjectBlueprint->name = "blueprint:";
	ObjectBlueprint->type = OTNATFUNC;
	ObjectBlueprint->value = (void*) &dnk_object_blueprint;
	HASH_ADD_KEYPTR(hh, Object->methods, ObjectBlueprint->name, strlen(ObjectBlueprint->name), ObjectBlueprint);
	
	obj* numberTimesObject = O();
	numberTimesObject->name = "times:";
	numberTimesObject->type = OTNATFUNC;
	numberTimesObject->value = (void*) &dnk_number_times;
	Number = O();
	Number->name = "Number";
	Number->value = "0";
	Number->type = OTNUMBER;
	HASH_ADD_KEYPTR(hh, World->properties, Number->name, strlen(Number->name), Number);
	HASH_ADD_KEYPTR(hh, Number->methods, numberTimesObject->name, strlen(numberTimesObject->name), numberTimesObject);
	
	obj* numberAdd = O();
	numberAdd->name = "+";
	numberAdd->type = OTNATFUNC;
	numberAdd->value = (void*) &dnk_number_add;
	HASH_ADD_KEYPTR(hh, Number->methods, numberAdd->name, strlen(numberAdd->name), numberAdd);
	
	obj* numberMin = O();
	numberMin->name = "-";
	numberMin->type = OTNATFUNC;
	numberMin->value = (void*) &dnk_number_minus;
	HASH_ADD_KEYPTR(hh, Number->methods, numberMin->name, strlen(numberMin->name), numberMin);
	
	obj* numberHiThan = O();
	numberHiThan->name = ">";
	numberHiThan->type = OTNATFUNC;
	numberHiThan->value = (void*) &dnk_number_higherThan;
	HASH_ADD_KEYPTR(hh, Number->methods, numberHiThan->name, strlen(numberHiThan->name), numberHiThan);
	
	obj* numberFactorial = O();
	numberFactorial->name = "factorial";
	numberFactorial->type = OTNATFUNC;
	numberFactorial->value = (void*) &dnk_number_factorial;
	HASH_ADD_KEYPTR(hh, Number->methods, numberFactorial->name, strlen(numberFactorial->name), numberFactorial);
	
	BoolX = O();
	BoolX->name = "Boolean";
	BoolX->type = OTBOOL;
	BoolX->value = "Boolean";
	HASH_ADD_KEYPTR(hh, World->properties, BoolX->name, strlen(BoolX->name), BoolX);
		
	obj* ifTrue = O();
	ifTrue->name = "ifTrue:";
	ifTrue->type = OTNATFUNC;
	ifTrue->value = (void*) &dnk_bool_iftrue;
	HASH_ADD_KEYPTR(hh, BoolX->methods, ifTrue->name, strlen(ifTrue->name), ifTrue);
	
	obj* ifFalse = O();
	ifFalse->name = "ifFalse:";
	ifFalse->type = OTNATFUNC;
	ifFalse->value = (void*) &dnk_bool_ifFalse;
	HASH_ADD_KEYPTR(hh, BoolX->methods, ifFalse->name, strlen(ifFalse->name), ifFalse);
	
	Nil = O();
	Nil->name = "Nil";
	Nil->type = OTNIL;
	Nil->value = "Nil";
	HASH_ADD_KEYPTR(hh, World->properties, Nil->name, strlen(Nil->name), Nil);
		
	obj* isNil = O();
	isNil->name = "isNil";
	isNil->type = OTNATFUNC;
	isNil->value = (void*) &dnk_nil_isnil;
	HASH_ADD_KEYPTR(hh, Nil->methods, isNil->name, strlen(isNil->name), isNil);
	
	
}

obj* dnk_send_message(obj* receiverObject, char* message, args* argumentList) {
	obj* methodObject = NULL;
	int antiCrash = 0;
	obj* searchObject = receiverObject;
	while(antiCrash++<100 && !methodObject) {
		HASH_FIND_STR(searchObject->methods, message, methodObject);
		if (methodObject) break;
		if (!searchObject->link) {
			break;
		}
		searchObject = searchObject->link;
	}
	
	if (!methodObject) {
		printf("Object will not respond to: %s\n", message);
		exit(1);
	}
	obj* result;
	if (methodObject->type == OTNATFUNC) {
		obj* (*funct)(obj* receiverObject, args* argumentList);
		funct = (void*) methodObject->value;
		result = (obj*) funct(receiverObject, argumentList);
	}
	
	if (methodObject->type == OTBLOCK) {
		//important! for messages to 'me', adjust the 'my' scope to the receiver itself.
		if (strcmp(receiverObject->name,"me")==0) {
			receiverObject = receiverObject->link;
		}
		result = dnk_block_run(methodObject, argumentList, receiverObject);
	}
	
	return result;
}

obj* dnk_assign_value(char* name, obj* o) {
	obj* object = O();
	object = o;
	object->name = name;
	dnk_set(object);
	return object;
}

obj* dnk_assign_value_to_my(char* name, obj* o) {
	obj* my = dnk_find("me");
	HASH_ADD_KEYPTR(hh, my->link->properties, name, strlen(name), o);
	return o;
}
