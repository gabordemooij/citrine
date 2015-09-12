#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <unistd.h>

#include "uthash.h"
#include "citrine.h"

obj* World = NULL;
obj* contexts[100];
int cid = 0;
obj* ctr_build_number(char* number);
obj* Object;
obj* CBlock;
obj* TextString;
obj* Number;
obj* BoolX;
obj* Console;
obj* Nil;
obj* GC;
obj* CMap;
obj* CArray;
obj* CFile;
obj* error;
obj* CSystem;
obj* CDice;
obj* CDog;
obj* CCoin;
obj* CCoffee;
int debug;

//measures the size of character
int utf8size(char c) {
	if ((c & UTF8_BYTE3) == UTF8_BYTE3) return 4;
	if ((c & UTF8_BYTE2) == UTF8_BYTE2) return 3;
	if ((c & UTF8_BYTE1) == UTF8_BYTE1) return 2;
	return 1;
}
//measures the length of an utf8 string in utf8 chars
long getutf8len(char* strval, long max) {
	long i;
	long j = 0;
	int s = 0;
	for(i = 0; i < max; i++) {
		s = utf8size(strval[i]);
		j += (s - 1);
	}
	return (i-j);
}

long getBytesUtf8(char* strval, long startByte, long lenUChar) {
	long i = 0;
	long bytes = 0;
	int s = 0;
	int x = 0;
	long index = 0;
	while(x < lenUChar) {
		index = startByte + i;
		char c = strval[index];
		s = utf8size(c);
		bytes = bytes + s;
		i = i + s;
		x ++;
	}
	return bytes;
}

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
   ctr_program_length=0;
   while( ( ch = fgetc(fp) ) != EOF ) prg[ctr_program_length++]=ch;
   fclose(fp);
   return prg;
}

void tree(tnode* ti, int indent) {
	if (indent>20) exit(1); 
	tlistitem* li = ti->nodes;
	tnode* t = li->node;
	while(1) {
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
		printf("%d:%s %s (vlen: %lu) \n", t->type, str, t->value, t->vlen);
		if (t->nodes) tree(t, indent + 1);
		if (!li->next) break; 
		li = li->next;
		t = li->node;
	}
}


int ctr_internal_object_is_equal(obj* object1, obj* object2) {
	
	if (object1->info.type == OTSTRING && object2->info.type == OTSTRING) {
		char* string1 = object1->value.svalue->value;
		char* string2 = object2->value.svalue->value;
		long len1 = object1->value.svalue->vlen;
		long len2 = object2->value.svalue->vlen;
		if (len1 != len2) return 0;
		int d = memcmp(string1, string2, len1);
		if (d==0) return 1;
		return 0;
	}
	
	if (object1->info.type == OTNUMBER && object2->info.type == OTNUMBER) {
		double num1 = object1->value.nvalue;
		double num2 = object2->value.nvalue;
		if (num1 == num2) return 1;
		return 0;
	}
	
	if (object1->info.type == OTBOOL && object2->info.type == OTBOOL) {
		int b1 = object1->value.bvalue;
		int b2 = object2->value.bvalue;
		if (b1 == b2) return 1;
		return 0;
	}
	
	if (object1 == object2) return 1;
	return 0;
		
}

obj* ctr_internal_object_find_property(obj* owner, obj* key, int is_method) {
	
//	printf("owner has %d props.\n", owner->properties->size);
	cmapitem* head;
	if (is_method) {
		if (owner->methods->size == 0) {
			return NULL;
		}
		head = owner->methods->head; 
	} else {
		if (owner->properties->size == 0) {
			return NULL;
		}
		head = owner->properties->head; 
	}
	
	while(head) {
		if (ctr_internal_object_is_equal(head->key, key)) {
			return head->value;
		}
		head = head->next;
	}
	return NULL;
}


void ctr_internal_object_delete_property(obj* owner, obj* key, int is_method) {
	cmapitem* head;
	if (is_method) {
		if (owner->methods->size == 0) {
			return;
		}
		head = owner->methods->head; 
	} else {
		if (owner->properties->size == 0) {
			return;
		}
		head = owner->properties->head; 
	}
	
	while(head) {
		if (ctr_internal_object_is_equal(head->key, key)) {
			if (head->next && head->prev) {
				head->next->prev = head->prev;
				head->prev->next = head->next;
			} else {
				if (head->next) {
					head->next->prev = NULL;
				}
				if (head->prev) {
					head->prev->next = NULL;
				}
			}
			if (is_method) {
				if (owner->methods->head == head) {
					if (head->next) {
						owner->methods->head = head->next;
					} else {
						owner->methods->head = NULL;
					}
				}
				owner->methods->size --; 
			} else {
				if (owner->properties->head == head) {
					if (head->next) {
						owner->properties->head = head->next;
					} else {
						owner->properties->head = NULL;
					}
				}
				owner->properties->size --;
			}
			return;
		}
		head = head->next;
	}
	return;
}

void ctr_internal_object_add_property(obj* owner, obj* key, obj* value, int m) {
	cmapitem* new_item = malloc(sizeof(cmapitem));
	cmapitem* current_head = NULL;
	new_item->key = key;
	new_item->value = value;
	new_item->next = NULL;
	new_item->prev = NULL;
	if (m) {
		if (owner->methods->size == 0) {
			owner->methods->head = new_item;
		} else {
			current_head = owner->methods->head;
			current_head->prev = new_item;
			new_item->next = current_head;
			owner->methods->head = new_item;
		}
		owner->methods->size ++;
	} else {
		if (owner->properties->size == 0) {
			owner->properties->head = new_item;
		} else {
			current_head = owner->properties->head;
			current_head->prev = new_item;
			new_item->next = current_head;
			owner->properties->head = new_item;
		}
		owner->properties->size ++;
	}
}

void ctr_internal_object_set_property(obj* owner, obj* key, obj* value, int is_method) {
	ctr_internal_object_delete_property(owner, key, is_method);
	ctr_internal_object_add_property(owner, key, value, is_method);
}

obj* ctr_internal_create_object(int type) {
	obj* o = malloc(sizeof(obj));
	o->properties = malloc(sizeof(ctr_map));
	o->methods = malloc(sizeof(ctr_map));
	
	o->properties->size = 0;
	o->methods->size = 0;
	o->properties->head = NULL;
	o->methods->head = NULL;
	o->info.type = type;
	
	if (type==OTBOOL) o->value.bvalue = 0;
	if (type==OTNUMBER) o->value.nvalue = 0;
	if (type==OTSTRING) {
		o->value.svalue = malloc(sizeof(cstr));
		o->value.svalue->value = "";
		o->value.svalue->vlen = 0;
	}
	CTR_REGISTER_OBJECT(o);
	return o;
	
}

void ctr_internal_create_func(obj* o, obj* key, void* f ) {
	obj* methodObject = ctr_internal_create_object(OTNATFUNC);
	methodObject->value.rvalue = (void*) f;
	ctr_internal_object_add_property(o, key, methodObject, 1);
}

obj* ctr_internal_cast2string( obj* o ) {
	if (o->info.type == OTSTRING) return o;
	else if (o->info.type == OTNIL) { return ctr_build_string("[Nil]", 5); }
	else if (o->info.type == OTBOOL && o->value.bvalue == 1) { return ctr_build_string("[True]", 6); }
	else if (o->info.type == OTBOOL && o->value.bvalue == 0) { return ctr_build_string("[False]", 7); }
	else if (o->info.type == OTNUMBER) {
		char* s = calloc(80, sizeof(char));
		CTR_CONVFP(s,o->value.nvalue);
		int slen = strlen(s);
		return ctr_build_string(s, slen);
	}
	else if (o->info.type == OTBLOCK) { return ctr_build_string("[Block]",7);}
	else if (o->info.type == OTOBJECT) { return ctr_build_string("[Object]",8);}
	return ctr_build_string("[?]", 3);
}

obj* ctr_internal_cast2bool( obj* o ) {
	if (o->info.type == OTBOOL) return o;
	if (o->info.type == OTNIL
		|| (o->info.type == OTNUMBER && o->value.nvalue == 0)
		|| (o->info.type == OTSTRING && o->value.svalue->vlen == 0)) return ctr_build_bool(0);
	return ctr_build_bool(1);
}


void ctr_open_context() {
	cid++;
	obj* context = ctr_internal_create_object(OTOBJECT);
	contexts[cid] = context;
}

void ctr_close_context() {
	if (cid == 0) return;
	cid--;
}

obj* ctr_find(obj* key) {
	int i = cid;
	obj* foundObject = NULL;
	while((i>-1 && foundObject == NULL)) {
		obj* context = contexts[i];
		foundObject = ctr_internal_object_find_property(context, key, 0);
		i--;
	}
	if (foundObject == NULL) { printf("Error, key not found: [%s].\n", key->value.svalue->value); exit(1); }
	return foundObject;
}

obj* ctr_find_in_my(obj* key) {
	obj* context = ctr_find(ctr_build_string("me",2));
	obj* foundObject = ctr_internal_object_find_property(context, key, 0);
	if (foundObject == NULL) { printf("Error, property not found: %s.\n", key->value.svalue->value); exit(1); }
	return foundObject;
}

void ctr_set(obj* key, obj* object) {
	obj* context = contexts[cid];
	ctr_internal_object_set_property(context, key, object, 0);
}

obj* ctr_build_bool(int truth) {
	obj* boolObject = ctr_internal_create_object(OTBOOL);
	if (truth) boolObject->value.bvalue = 1; else boolObject->value.bvalue = 0;
	boolObject->info.type = OTBOOL;
	boolObject->link = BoolX;
	return boolObject;
}


obj* ctr_console_write(obj* myself, args* argumentList) {
	obj* argument1 = argumentList->object;
	obj* strObject = ctr_internal_cast2string(argument1);
	fwrite(strObject->value.svalue->value, sizeof(char), strObject->value.svalue->vlen, stdout);
	return myself;
}

obj* ctr_block_run(obj* myself, args* argList, obj* my) {
	//obj* selfRef = ctr_internal_create_object(OTOBJECT);
	//ASSIGN_STRING(selfRef->value.svalue, value, "[self]", 6);
	//selfRef->link = my;
	obj* result;
	
	tnode* node = myself->value.block;
	
	//obj* thisBlock = ctr_internal_create_object(OTBLOCK);
	
	//ASSIGN_STRING(thisBlock,name,"thisBlock",9);
	//thisBlock->link = myself;
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

obj* ctr_bool_iftrue(obj* myself, args* argumentList) {
	if (myself->value.bvalue) {
		obj* codeBlock = argumentList->object;
		args* arguments = CTR_CREATE_ARGUMENT();
		arguments->object = myself;
		return ctr_block_run(codeBlock, arguments, myself);
	}
	return myself;
}

obj* ctr_bool_ifFalse(obj* myself, args* argumentList) {
	if (!myself->value.bvalue) {
		obj* codeBlock = argumentList->object;
		args* arguments = CTR_CREATE_ARGUMENT();
		arguments->object = myself;
		return ctr_block_run(codeBlock, arguments, myself);
	}
	return myself;
}

obj* ctr_bool_opposite(obj* myself, args* argumentList) {
	if (myself->value.bvalue == 0) myself->value.bvalue = 1; else myself->value.bvalue = 0;
	return myself;
}

obj* ctr_bool_and(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	if (argumentList->object->info.type != OTBOOL) {
		printf("Argument of binary message && must be a boolean.\n"); exit(1);
	}
	return ctr_build_bool((myself->value.bvalue && argumentList->object->value.bvalue));
}

obj* ctr_bool_or(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	if (argumentList->object->info.type != OTBOOL) {
		printf("Argument of binary message || must be a boolean.\n"); exit(1);
	}
	return ctr_build_bool((myself->value.bvalue || argumentList->object->value.bvalue));
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

//create number from \0 terminated string
obj* ctr_build_number(char* n) {
	obj* numberObject = ctr_internal_create_object(OTNUMBER);
	numberObject->value.nvalue = atof(n);
	numberObject->link = Number;
	return numberObject;
}

obj* ctr_object_make() {
	obj* objectInstance = NULL;
	objectInstance = ctr_internal_create_object(OTOBJECT);
	objectInstance->link = Object;
	return objectInstance;
}

obj* ctr_object_method_does(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	if (!argumentList->next) {
		printf("Missing argument 2\n"); exit(1);
	}
	obj* methodName = argumentList->object;
	if (methodName->info.type != OTSTRING) {
		printf("Expected argument method: to be of type string.\n");
		exit(1);
	}
	args* nextArgument = argumentList->next;
	if (!nextArgument->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	obj* methodBlock = nextArgument->object;
	if (methodBlock->info.type != OTBLOCK) {
		printf("Expected argument does: to be of type block.\n");
		exit(1);
	}
	ctr_internal_object_add_property(myself, methodName, methodBlock, 1);
	return myself;
}


obj* ctr_object_override_does(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	if (!argumentList->next) {
		printf("Missing argument 2\n"); exit(1);
	}
	obj* methodName = argumentList->object;
	if (methodName->info.type != OTSTRING) {
		printf("Expected argument method: to be of type string.\n");
		exit(1);
	}
	args* nextArgument = argumentList->next;
	if (!nextArgument->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	obj* methodBlock = nextArgument->object;
	if (methodBlock->info.type != OTBLOCK) {
		printf("Expected argument does: to be of type block.\n");
		exit(1);
	}
	obj* overriddenMethod = ctr_internal_object_find_property(myself, methodName, 1);
	if (overriddenMethod == NULL) {
		printf("Cannot override. No such method.");
		exit(1);
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

obj* ctr_object_respond(obj* myself, args* argumentList) {
	return myself;
}

obj* ctr_object_blueprint(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	obj* other = argumentList->object;
	if (other->info.type != OTOBJECT) {
		printf("Expected argument method: to be of type object.\n");
		exit(1);
	}
	myself->link = other;
	return myself;
}

obj* ctr_build_string(char* stringValue, long size) {
	obj* stringObject = ctr_internal_create_object(OTSTRING);
	ASSIGN_STRING(stringObject->value.svalue, value, stringValue, size);
	stringObject->value.svalue->vlen = size;
	stringObject->link = TextString;
	return stringObject;
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

obj* ctr_build_nil() {	
	return Nil;
}

obj* ctr_nil_isnil(obj* myself, args* argumentList) {
	obj* truth;
	if ((myself->info.type == OTNIL)) {
		truth = ctr_build_bool(1);
	} else {
		truth = ctr_build_bool(0);
	}
	return truth;
}

void ctr_gc_mark(obj* object) {
	/*obj* item;
	obj* tmp;
	HASH_ITER(hh, object->properties, item, tmp) {
		if (item->info.sticky) continue;
		item->info.mark = 1;
		item->info.sticky = 0;
		ctr_gc_mark(item);
	}
	HASH_ITER(hh, object->methods, item, tmp) {
		if (item->info.sticky) continue;
		item->info.mark = 1;
		item->info.sticky = 0;
		ctr_gc_mark(item);
	}*/
}

void ctr_gc_sweep() {
/*	obj** q = &ctr_first_object;
	while(*q) {
		if ((*q)->info.mark==0 && (*q)->info.sticky==0){
			obj* u = *q;
			*q = u->next;
			int z;
			for(z =0; z <= cid; z++) {
				ctr_internal_object_find_property_mapitem_by_value(contexts[z]->properties, u);
				//ctr_internal_object_delete_property(contexts[z], u);
			}
			free(u);
		} else {
			if ((*q)->info.mark == 1) {
				(*q)->info.mark = 0;
			}
			q = &(*q)->next;
		}
	}*/
}

void ctr_gc_collect (obj* myself, args* argumentList) {
	/*obj* context = contexts[cid];
	int oldcid = cid;
	while(cid > -1) {
		ctr_gc_mark(context);
		cid --;
		context = contexts[cid];
	}
	ctr_gc_sweep();
	cid = oldcid;*/
}


obj* ctr_map_put(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	if (!argumentList->next) {
		printf("Missing argument 2\n"); exit(1);
	}
	obj* putValue = argumentList->object;
	args* nextArgument = argumentList->next;
	if (!nextArgument->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	obj* putKey = nextArgument->object;
	
	char* key;
	long keyLen;
	
	if (putKey->info.type == OTSTRING) {
		key = calloc(putKey->value.svalue->vlen, sizeof(char));
		keyLen = putKey->value.svalue->vlen;
		memcpy(key, putKey->value.svalue->value, keyLen);
	} else {
		printf("Map key needs to be string.\n");
		exit(1);
	}
	ctr_internal_object_delete_property(myself, ctr_build_string(key, keyLen), 0);
	ctr_internal_object_add_property(myself, ctr_build_string(key, keyLen), putValue, 0);
	
    return myself;
}

obj* ctr_map_get(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	obj* searchKey = argumentList->object;
	if (searchKey->info.type != OTSTRING) {
		printf("Expected argument at: to be of type string.\n");
		exit(1);
	}
	obj* foundObject = ctr_internal_object_find_property(myself, searchKey, 0);
	if (foundObject == NULL) foundObject = ctr_build_nil();
	return foundObject;
}

obj* ctr_map_count(obj* myself) {

	
	return ctr_build_number_from_float( myself->properties->size );
}


obj* ctr_map_each(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	obj* block = argumentList->object;
	if (block->info.type != OTBLOCK) { printf("Expected code block."); exit(1); }
	block->info.sticky = 1; //mark as sticky
	cmapitem* m = myself->properties->head;
	while(m) {
		args* arguments = CTR_CREATE_ARGUMENT();
		arguments->object = m->value;
		ctr_block_run(block, arguments, myself);
		m = m->next;
	}
	block->info.mark = 0;
	block->info.sticky = 0;
	return myself;
}

obj* ctr_array_new(obj* myclass) {
	obj* s = ctr_internal_create_object(OTARRAY);
	s->link = CArray;
	s->value.avalue = (carray*) malloc(sizeof(carray));
	s->value.avalue->length = 1;
	s->value.avalue->elements = (obj**) malloc(sizeof(obj*)*1);
	s->value.avalue->head = 0;
	s->info.flagb = 1;
	//printf("%d\n", s->value.avalue->length);
	return s;
}

obj* ctr_array_push(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	if (myself->value.avalue->length <= (myself->value.avalue->head + 1)) {
		myself->value.avalue->length = myself->value.avalue->length * 3;
		myself->value.avalue->elements = (obj**) realloc(myself->value.avalue->elements, (sizeof(obj*) * (myself->value.avalue->length)));
	}
	obj* pushValue = argumentList->object;
	int size = sizeof(obj*);
	int offset = (myself->value.avalue->head * size);
	long nbase = myself->value.avalue->elements;
	long locationa = nbase + offset;
	*((obj**)locationa) = pushValue;
	myself->value.avalue->head++;
	return myself;
}

obj* ctr_array_get(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	obj* getIndex = argumentList->object;
	if (getIndex->info.type != OTNUMBER) {
		printf("Index must be number.\n"); exit(1);
	}
	int i = (int) getIndex->value.nvalue;
	if (myself->value.avalue->head < i || i < 0) {
		printf("Index out of bounds.\n"); exit(1);
	}
	return *( (obj**) ((long)myself->value.avalue->elements + (i * sizeof(obj*))) );
}

obj* ctr_array_put(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing argument 1\n"); exit(1);
	}
	if (!argumentList->next) {
		printf("Missing argument 2\n"); exit(1);
	}
	obj* putValue = argumentList->object;
	obj* putIndex = argumentList->next->object;
	if (putIndex->info.type != OTNUMBER) {
		printf("Index must be number.\n"); exit(1);
	}
	int i = (int) putIndex->value.nvalue;
	if (myself->value.avalue->head < i || i < 0) {
		printf("Index out of bounds.\n"); exit(1);
	}
	*( (obj**) ((long)myself->value.avalue->elements + (i * sizeof(obj*))) ) = putValue;
	return myself;
}

//@todo dont forget to gc arrays, they might hold refs to objects!
obj* ctr_array_pop(obj* myself) {
	if (myself->value.avalue->length < myself->value.avalue->head) {
		return Nil;
	}
	myself->value.avalue->head--;
	return (obj*) *((obj**)( (long) myself->value.avalue->elements + (myself->value.avalue->head * sizeof(obj*))  ));
}

obj* ctr_array_count(obj* myself) {
	double d = 0;
	d = (double) myself->value.avalue->head;
	return ctr_build_number_from_float( d );
}


obj* ctr_file_new(obj* myself, args* argumentList) {
	obj* s = ctr_object_make();
	s->info.type = OTMISC;
	s->link = myself;
	s->info.flagb = 1;
	if (!argumentList->object) {
		printf("Missing argument\n");
		exit(1);
	}
	s->value.rvalue = malloc(sizeof(cres));
	s->value.rvalue->type = 1;
	obj* pathObject = ctr_internal_create_object(OTSTRING);
	pathObject->info.type = OTSTRING;
	pathObject->value.svalue = (cstr*) malloc(sizeof(cstr));
	pathObject->value.svalue->value = (char*) malloc(sizeof(char) * argumentList->object->value.svalue->vlen);
	memcpy(pathObject->value.svalue->value, argumentList->object->value.svalue->value, argumentList->object->value.svalue->vlen);
	pathObject->value.svalue->vlen = argumentList->object->value.svalue->vlen;
	ctr_internal_object_add_property(s, ctr_build_string("path",4), pathObject, 0);
	return s;
}

obj* ctr_file_path(obj* myself) {
	obj* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	if (path == NULL) return Nil;
	return path;
}

obj* ctr_file_read(obj* myself) {
	obj* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	if (path == NULL) return Nil;
	long vlen = path->value.svalue->vlen;
	char* pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	FILE* f = fopen(pathString, "rb");
	if (!f) {
		printf("Unable to open file!\n");
		exit(1);
	}
	char *buffer;
	unsigned long fileLen;
	fseek(f, 0, SEEK_END);
	fileLen=ftell(f);
	fseek(f, 0, SEEK_SET);
	buffer=(char *)malloc(fileLen+1);
	if (!buffer){
		printf("Out of memory\n");
		fclose(f);exit(1);	
	}
	fread(buffer, fileLen, 1, f);
	fclose(f);
	obj* str = ctr_build_string(buffer, fileLen);
	free(buffer);
	//free(f);
	return str;
}

obj* ctr_file_write(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing string argument to write to file.\n");
		exit(1);
	}
	obj* str = argumentList->object;
	if (str->info.type != OTSTRING) {
		printf("First argument must be string\n");
		exit(1);
	}
	obj* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	if (path == NULL) return Nil;
	long vlen = path->value.svalue->vlen;
	char* pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	FILE* f = fopen(pathString, "wb+");
	if (!f) {
		printf("Unable to open file!\n");
		exit(1);
	}
	fwrite(str->value.svalue->value, sizeof(char), str->value.svalue->vlen, f);
	fclose(f);
	//free(f);
	return myself;
}

obj* ctr_file_append(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing string argument to write to file.\n");
		exit(1);
	}
	obj* str = argumentList->object;
	if (str->info.type != OTSTRING) {
		printf("First argument must be string\n");
		exit(1);
	}
	obj* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	if (path == NULL) return Nil;
	long vlen = path->value.svalue->vlen;
	char* pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	FILE* f = fopen(pathString, "ab+");
	if (!f) {
		printf("Unable to open file!\n");
		exit(1);
	}
	fwrite(str->value.svalue->value, sizeof(char), str->value.svalue->vlen, f);
	fclose(f);
	return myself;
}

obj* ctr_file_exists(obj* myself) {
	obj* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	if (path == NULL) return ctr_build_bool(0);
	long vlen = path->value.svalue->vlen;
	char* pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	FILE* f = fopen(pathString, "r");
	int exists = (f != NULL );
	if (f) {
		fclose(f);
		//free(f);
	}
	return ctr_build_bool(exists);
}

obj* ctr_file_delete(obj* myself) {
	obj* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	if (path == NULL) return ctr_build_bool(0);
	long vlen = path->value.svalue->vlen;
	char* pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	int r = remove(pathString);
	if (r!=0) {
		printf("Cant delete file.");
		exit(1);
	}
	return myself;
}

obj* ctr_sys_call(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("No system argument.\n");
		exit(1);
	}
	if (argumentList->object->info.type!=OTSTRING) {
		printf("Argument of system call needs to be string\n");
		exit(1);
	}
	long vlen = argumentList->object->value.svalue->vlen;
	char* comString = malloc(vlen + 1);
	memcpy(comString, argumentList->object->value.svalue->value, vlen);
	memcpy(comString+vlen,"\0",1);
	int r = system(comString);
	return ctr_build_number_from_float( (float) r );
}

obj* ctr_file_size(obj* myself) {
	obj* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	if (path == NULL) return ctr_build_number_from_float(0);
	long vlen = path->value.svalue->vlen;
	char* pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	FILE* f = fopen(pathString, "r");
	if (f == NULL) return ctr_build_number_from_float(0);
	int prev = ftell(f);
    fseek(f, 0L, SEEK_END);
    int sz=ftell(f);
    fseek(f,prev,SEEK_SET); //go back to where we were
    if (f) {
		fclose(f);
		//free(f);
	}
    return ctr_build_number_from_float( (double) sz );
}

obj* ctr_dice_sides(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("No number of sides argument.\n");
		exit(1);
	}
	if (argumentList->object->info.type!=OTNUMBER) {
		printf("Argument sides needs to be number\n");
		exit(1);
	}
	
	
	return ctr_build_number_from_float((rand() % ((int)argumentList->object->value.nvalue)));
}

obj* ctr_dice_throw(obj* myself) {
	return ctr_build_number_from_float((rand() % 6));
}

obj* ctr_coin_flip(obj* myself) {
	return ctr_build_bool((rand() % 2));
}

obj* ctr_dog_fetch_argument(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("No number of arg argument.\n");
		exit(1);
	}
	if (argumentList->object->info.type!=OTNUMBER) {
		printf("Argument argNo needs to be number\n");
		exit(1);
	}
	int n = (int) argumentList->object->value.nvalue;
	if (n >= __argc) return Nil;
	return ctr_build_string(__argv[n], strlen(__argv[n]));
}

obj* ctr_dog_num_of_args(obj* myself) {
	return ctr_build_number_from_float( __argc );
}

obj* ctr_coffee_brew(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("No brew argument.\n");
		exit(1);
	}
	if (argumentList->object->info.type!=OTNUMBER) {
		printf("Argument brew to be number\n");
		exit(1);
	}
	int n = (int) argumentList->object->value.nvalue;
	sleep(n);
	return myself;
}

void ctr_initialize_world() {
	
	srand((unsigned)time(NULL));
	CTR_INIT_HEAD_OBJECT();

	World = ctr_internal_create_object(OTOBJECT);
	World->info.mark = 0;
	World->info.sticky = 1;
	contexts[0] = World;
	
	Object = ctr_internal_create_object(OTOBJECT);
	ctr_internal_create_func(Object, ctr_build_string("new", 3), &ctr_object_make);
	ctr_internal_create_func(Object, ctr_build_string("method:does:", 12), &ctr_object_method_does);
	ctr_internal_create_func(Object, ctr_build_string("on:do:", 6), &ctr_object_method_does);
	ctr_internal_create_func(Object, ctr_build_string("override:does:", 14), &ctr_object_override_does);
	ctr_internal_create_func(Object, ctr_build_string("basedOn:", 8), &ctr_object_blueprint);
	ctr_internal_create_func(Object, ctr_build_string("respondTo:", 10), &ctr_object_respond);
	ctr_internal_create_func(Object, ctr_build_string("respondTo:with:", 15), &ctr_object_respond);
	ctr_internal_create_func(Object, ctr_build_string("respondTo:with:and:", 19), &ctr_object_respond);
	ctr_internal_create_func(Object, ctr_build_string("new", 3), &ctr_object_make);
	Object->link = NULL;
	Object->info.mark = 0;
	Object->info.sticky = 1;
	ctr_internal_object_add_property(World, ctr_build_string("Object", 6), Object, 0);

	Console = ctr_internal_create_object(OTOBJECT);
	ctr_internal_create_func(Console, ctr_build_string("write:", 6), &ctr_console_write);
	ctr_internal_object_add_property(World, ctr_build_string("Pen", 3), Console, 0);
	Console->link = Object;
	Console->info.mark = 0;
	Console->info.sticky = 1;
	Console->info.flagb = 1;
	
	
	Number = ctr_internal_create_object(OTNUMBER);
	ctr_internal_create_func(Number, ctr_build_string("+", 1), &ctr_number_add);
	ctr_internal_create_func(Number, ctr_build_string("inc:",4), &ctr_number_inc);
	ctr_internal_create_func(Number, ctr_build_string("-",1), &ctr_number_minus);
	ctr_internal_create_func(Number, ctr_build_string("dec:",4), &ctr_number_dec);
	ctr_internal_create_func(Number, ctr_build_string("*",1),&ctr_number_multiply);
	ctr_internal_create_func(Number, ctr_build_string("mul:",4),&ctr_number_mul);
	ctr_internal_create_func(Number, ctr_build_string("/",1), &ctr_number_divide);
	ctr_internal_create_func(Number, ctr_build_string("div:",4),&ctr_number_div);
	ctr_internal_create_func(Number, ctr_build_string(">",1),&ctr_number_higherThan);
	ctr_internal_create_func(Number, ctr_build_string(">=",2),&ctr_number_higherEqThan);
	ctr_internal_create_func(Number, ctr_build_string("<",1),&ctr_number_lowerThan);
	ctr_internal_create_func(Number, ctr_build_string("<=",2),&ctr_number_lowerEqThan);
	ctr_internal_create_func(Number, ctr_build_string("==",2),&ctr_number_eq);
	ctr_internal_create_func(Number, ctr_build_string("!=",2),&ctr_number_neq);
	ctr_internal_create_func(Number, ctr_build_string("times:",6),&ctr_number_times);
	ctr_internal_create_func(Number, ctr_build_string("factorial",9),&ctr_number_factorial);
	ctr_internal_create_func(Number, ctr_build_string("between:and:",12),&ctr_number_between);
	ctr_internal_object_add_property(World, ctr_build_string("Number", 6), Number, 0);
	Number->link = Object;
	Number->info.mark = 0;
	Number->info.sticky = 1;
	
	
	CDice = ctr_internal_create_object(OTOBJECT);
	ctr_internal_create_func(CDice, ctr_build_string("roll", 4), &ctr_dice_throw);
	ctr_internal_create_func(CDice, ctr_build_string("rollWithSides:", 14), &ctr_dice_sides);
	ctr_internal_object_add_property(World, ctr_build_string("Dice", 4), CDice, 0);
	CDice->link = Object;
	CDice->info.mark = 0;
	CDice->info.sticky = 1;
	
	CCoin = ctr_internal_create_object(OTOBJECT);
	ctr_internal_create_func(CCoin, ctr_build_string("flip", 4), &ctr_coin_flip);
	ctr_internal_object_add_property(World, ctr_build_string("Coin", 4), CCoin, 0);
	CCoin->link = Object;
	CCoin->info.mark = 0;
	CCoin->info.sticky = 1;
	
	CCoffee = ctr_internal_create_object(OTOBJECT);
	ctr_internal_create_func(CCoffee, ctr_build_string("brew:", 5), &ctr_coffee_brew);
	ctr_internal_object_add_property(World, ctr_build_string("CoffeePot", 9), CCoffee, 0);
	CCoffee->link = Object;
	CCoffee->info.mark = 0;
	CCoffee->info.sticky = 1;
	
	
	CDog = ctr_internal_create_object(OTOBJECT);
	ctr_internal_create_func(CDog, ctr_build_string("fetchArg:", 9), &ctr_dog_fetch_argument);
	ctr_internal_create_func(CDog, ctr_build_string("fetchArgCount:", 13), &ctr_dog_num_of_args);
	ctr_internal_create_func(CDog, ctr_build_string("trick:", 6), &ctr_sys_call);
	ctr_internal_object_add_property(World, ctr_build_string("Dog", 3), CDog, 0);
	CDog->link = Object;
	CDog->info.mark = 0;
	CDog->info.sticky = 1;
	
	TextString = ctr_internal_create_object(OTSTRING);
	ctr_internal_create_func(TextString, ctr_build_string("printBytes", 10), &ctr_string_printbytes);
	ctr_internal_create_func(TextString, ctr_build_string("bytes", 5), &ctr_string_bytes);
	ctr_internal_create_func(TextString, ctr_build_string("length", 6), &ctr_string_length);
	ctr_internal_create_func(TextString, ctr_build_string("from:to:", 8), &ctr_string_fromto);
	ctr_internal_create_func(TextString, ctr_build_string("+", 1), &ctr_string_concat);
	ctr_internal_create_func(TextString, ctr_build_string("==", 2), &ctr_string_eq);
	ctr_internal_object_add_property(World, ctr_build_string("String", 6), TextString, 0);
	TextString->link = Object;
	TextString->info.mark = 0;
	TextString->info.sticky = 1;
		
	CMap = ctr_internal_create_object(OTOBJECT);
	ctr_internal_create_func(CMap, ctr_build_string("put:at:", 7), &ctr_map_put);
	ctr_internal_create_func(CMap, ctr_build_string("at:", 3), &ctr_map_get);
	ctr_internal_create_func(CMap, ctr_build_string("count", 5), &ctr_map_count);
	ctr_internal_create_func(CMap, ctr_build_string("each:", 5), &ctr_map_each);
	ctr_internal_object_add_property(World, ctr_build_string("Map", 3), CMap, 0);
	CMap->link = Object;
	CMap->info.mark = 0;
	CMap->info.sticky = 1;
	
	
	CArray = ctr_internal_create_object(OTARRAY);
	ctr_internal_create_func(CArray, ctr_build_string("new", 3), &ctr_array_new);
	ctr_internal_create_func(CArray, ctr_build_string("push:", 5), &ctr_array_push);
	ctr_internal_create_func(CArray, ctr_build_string("count", 5), &ctr_array_count);
	ctr_internal_create_func(CArray, ctr_build_string("pop", 3), &ctr_array_pop);
	ctr_internal_create_func(CArray, ctr_build_string("at:", 3), &ctr_array_get);
	ctr_internal_create_func(CArray, ctr_build_string("put:at:", 7), &ctr_array_put);
	ctr_internal_object_add_property(World, ctr_build_string("Array", 5), CArray, 0);
	CArray->link = Object;
	CArray->info.mark = 0;
	CArray->info.sticky = 1;
	
	CFile = ctr_internal_create_object(OTOBJECT);
	ctr_internal_create_func(CFile, ctr_build_string("new:", 4), &ctr_file_new);
	ctr_internal_create_func(CFile, ctr_build_string("path", 4), &ctr_file_path);
	ctr_internal_create_func(CFile, ctr_build_string("read", 4), &ctr_file_read);
	ctr_internal_create_func(CFile, ctr_build_string("write:", 6), &ctr_file_write);
	ctr_internal_create_func(CFile, ctr_build_string("append:", 7), &ctr_file_append);
	ctr_internal_create_func(CFile, ctr_build_string("exists", 6), &ctr_file_exists);
	ctr_internal_create_func(CFile, ctr_build_string("size", 4), &ctr_file_size);
	ctr_internal_create_func(CFile, ctr_build_string("delete", 6), &ctr_file_delete);
	ctr_internal_object_add_property(World, ctr_build_string("File", 4), CFile, 0);
	CFile->link = Object;
	CFile->info.mark = 0;
	CFile->info.sticky = 1;
	
	
	CBlock = ctr_internal_create_object(OTBLOCK);
	ctr_internal_create_func(CBlock, ctr_build_string("run", 3), &ctr_block_run);
	ctr_internal_create_func(CBlock, ctr_build_string("error:", 6), &ctr_block_error);
	ctr_internal_create_func(CBlock, ctr_build_string("catch:", 6), &ctr_block_catch);
	ctr_internal_object_add_property(World, ctr_build_string("CodeBlock", 9), CBlock, 0);
	CBlock->link = Object;
	CBlock->info.mark = 0;
	CBlock->info.sticky = 1;
	
	
	BoolX = ctr_internal_create_object(OTBOOL);
	ctr_internal_create_func(BoolX, ctr_build_string("ifTrue:", 7), &ctr_bool_iftrue);
	ctr_internal_create_func(BoolX, ctr_build_string("ifFalse:", 8), &ctr_bool_ifFalse);
	ctr_internal_create_func(BoolX, ctr_build_string("opposite", 8), &ctr_bool_opposite);
	ctr_internal_create_func(BoolX, ctr_build_string("&&", 2), &ctr_bool_and);
	ctr_internal_create_func(BoolX, ctr_build_string("||", 2), &ctr_bool_or);
	ctr_internal_object_add_property(World, ctr_build_string("Boolean", 7), BoolX, 0);
	BoolX->link = Object;
	BoolX->info.mark = 0;
	BoolX->info.sticky = 1;
	
	
	GC = ctr_internal_create_object(OTOBJECT);
	ctr_internal_create_func(GC, ctr_build_string("sweep", 5), &ctr_gc_collect);
	ctr_internal_object_add_property(World, ctr_build_string("Broom", 5), GC, 0);
	GC->link = Object;
	GC->info.mark = 0;
	GC->info.sticky = 1;
	
	Nil = ctr_internal_create_object(OTNIL);
	ctr_internal_object_add_property(World, ctr_build_string("Nil", 3), Nil, 0);
	ctr_internal_create_func(Nil, ctr_build_string("isNil", 5), &ctr_nil_isnil);
	Nil->link = Object;
	Nil->info.mark = 0;
	Nil->info.sticky = 1;
}

obj* ctr_send_message(obj* receiverObject, char* message, long vlen, args* argumentList) {
	
	if (error != NULL) return NULL; //Error mode, ignore subsequent messages until resolved.
	obj* methodObject = NULL;
	obj* searchObject = receiverObject;
	while(!methodObject) {
		methodObject = ctr_internal_object_find_property(searchObject, ctr_build_string(message, vlen), 1);
		if (methodObject) break;
		if (!searchObject->link) break;
		searchObject = searchObject->link;
	}
	
	if (!methodObject) {
		args* argCounter = argumentList;
		int argCount = 0;
		while(argCounter->next && argCount < 4) {
			argCounter = argCounter->next;
			argCount ++;
		}
		args* mesgArgument = CTR_CREATE_ARGUMENT();
		mesgArgument->object = ctr_build_string(message, vlen);
		mesgArgument->next = argumentList;
		if (argCount == 0 || argCount > 2) {
			return ctr_send_message(receiverObject, "respondTo:", 10,  mesgArgument);
		} else if (argCount == 1) {
			return ctr_send_message(receiverObject, "respondTo:with:", 15,  mesgArgument);
		} else if (argCount == 2) {
			return ctr_send_message(receiverObject, "respondTo:with:and:", 19,  mesgArgument);
		}
	}
	obj* result;
	
	
	if (methodObject->info.type == OTNATFUNC) {
		
		obj* (*funct)(obj* receiverObject, args* argumentList);
		funct = (void*) methodObject->value.block;
		result = (obj*) funct(receiverObject, argumentList);
	}
	if (methodObject->info.type == OTBLOCK) {
		result = ctr_block_run(methodObject, argumentList, receiverObject);
	}	
	return result;
}

obj* ctr_assign_value(obj* key, obj* o) {
	obj* object;
	if (o->info.type == OTOBJECT || o->info.type == OTMISC || o->info.type == OTARRAY) {
		ctr_set(key, o);
	} else {
		object = ctr_internal_create_object(o->info.type);
		object->properties = o->properties;
		object->methods = o->methods;
		object->link = o->link;
		ctr_set(key, object);
	}
     //depending on type, copy specific value
    if (o->info.type == OTBOOL) {
		object->value.bvalue = o->value.bvalue;
	 } else if (o->info.type == OTNUMBER) {
		object->value.nvalue = o->value.nvalue;
	 } else if (o->info.type == OTSTRING) {
		object->value.svalue = malloc(sizeof(cstr));
		object->value.svalue->value = malloc(sizeof(char)*o->value.svalue->vlen);
		memcpy(object->value.svalue->value, o->value.svalue->value,o->value.svalue->vlen);
		object->value.svalue->vlen = o->value.svalue->vlen;
	 } else if (o->info.type == OTBLOCK) {
		object->value.block = o->value.block;
	 } else if (o->info.type == OTARRAY) {
		/*object->value.avalue = malloc(sizeof(carray));
		object->value.avalue->elements = malloc(o->value.avalue->length*sizeof(obj*));
		object->value.avalue->length = o->value.avalue->length;
		int i;
		for (i = 0; i < object->value.avalue->head; i++) {
			*(object->value.avalue->elements+(i*sizeof(obj*))) = *(o->value.avalue->elements+(i*sizeof(obj*)));
		}
		object->value.avalue->head = o->value.avalue->head;*/
	 }
	return object;
}

obj* ctr_assign_value_to_my(obj* key, obj* o) {
	obj* object;
	obj* my = ctr_find(ctr_build_string("me", 2));
	if (o->info.type == OTOBJECT || o->info.type == OTMISC) {
		ctr_internal_object_add_property(my, key, o, 0);
	} else {
		object = ctr_internal_create_object(o->info.type);
		object->properties = o->properties;
		object->methods = o->methods;
		object->link = o->link;
		ctr_internal_object_add_property(my, key, object, 0);
	}
     //depending on type, copy specific value
    if (o->info.type == OTBOOL) {
		object->value.bvalue = o->value.bvalue;
	 } else if (o->info.type == OTNUMBER) {
		object->value.nvalue = o->value.nvalue;
	 } else if (o->info.type == OTSTRING) {
		object->value.svalue = malloc(sizeof(cstr));
		object->value.svalue->value = malloc(sizeof(char)*o->value.svalue->vlen);
		memcpy(object->value.svalue->value, o->value.svalue->value,o->value.svalue->vlen);
		object->value.svalue->vlen = o->value.svalue->vlen;
	 } else if (o->info.type == OTBLOCK) {
		object->value.block = o->value.block;
	 } else if (o->info.type == OTARRAY) {
		object->value.avalue = malloc(sizeof(carray));
		object->value.avalue->elements = malloc(o->value.avalue->length*sizeof(obj*));
		object->value.avalue->length = o->value.avalue->length;
		int i;
		for (i = 0; i < object->value.avalue->length; i++) {
			object->value.avalue->elements = *(o->value.avalue->elements+(i*sizeof(obj*)));
		}
		object->value.avalue->head = o->value.avalue->head;
		object->info.flagb = 1;
	 }

	return object;
}

