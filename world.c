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
 * ReadFile
 *
 * Reads in an entire file.
 */
char* ctr_internal_readf(char* file_name) {
   char* prg;
   char ch;
   int prev;
   int sz;
   FILE* fp;
   fp = fopen(file_name,"r");
   if( fp == NULL )
   {
      printf("Error while opening the file.\n");
      exit(1);
   }
   prev = ftell(fp);
   fseek(fp,0L,SEEK_END);
   sz = ftell(fp);
   fseek(fp,prev,SEEK_SET);
   prg = malloc((sz+4)*sizeof(char)); /* add 4 bytes, 3 for optional closing sequence verbatim mode and one lucky byte! */
   ctr_program_length=0;
   while( ( ch = fgetc(fp) ) != EOF ) prg[ctr_program_length++]=ch;
   fclose(fp);
   return prg;
}

/**
 * InternalObjectIsEqual
 *
 * Detemines whether two objects are identical.
 */
int ctr_internal_object_is_equal(ctr_object* object1, ctr_object* object2) {
	char* string1;
	char* string2;
	ctr_size len1;
	ctr_size len2;
	ctr_size d;
	if (object1->info.type == CTR_OBJECT_TYPE_OTSTRING && object2->info.type == CTR_OBJECT_TYPE_OTSTRING) {
		string1 = object1->value.svalue->value;
		string2 = object2->value.svalue->value;
		len1 = object1->value.svalue->vlen;
		len2 = object2->value.svalue->vlen;
		if (len1 != len2) return 0;
		d = memcmp(string1, string2, len1);
		if (d==0) return 1;
		return 0;
	}
	if (object1->info.type == CTR_OBJECT_TYPE_OTNUMBER && object2->info.type == CTR_OBJECT_TYPE_OTNUMBER) {
		ctr_number num1 = object1->value.nvalue;
		ctr_number num2 = object2->value.nvalue;
		if (num1 == num2) return 1;
		return 0;
	}
	if (object1->info.type == CTR_OBJECT_TYPE_OTBOOL && object2->info.type == CTR_OBJECT_TYPE_OTBOOL) {
		int b1 = object1->value.bvalue;
		int b2 = object2->value.bvalue;
		if (b1 == b2) return 1;
		return 0;
	}
	if (object1 == object2) return 1;
	return 0;		
}

/**
 * InternalObjectIndexHash
 *
 * Given an object, this function will calculate a hash to speed-up
 * lookup.
 */
uint64_t ctr_internal_index_hash(ctr_object* key) {
	ctr_object* stringKey = ctr_internal_cast2string(key);
	return siphash24(stringKey->value.svalue->value, stringKey->value.svalue->vlen, CtrHashKey);
}

/**
 * InternalObjectFindProperty
 *
 * Finds property in object.
 */
ctr_object* ctr_internal_object_find_property(ctr_object* owner, ctr_object* key, int is_method) {

	ctr_mapitem* head;
	uint64_t hashKey = ctr_internal_index_hash(key);

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
		if ((hashKey == head->hashKey) && ctr_internal_object_is_equal(head->key, key)) {
			return head->value;
		}
		head = head->next;
	}
	return NULL;
}


/**
 * InternalObjectDeleteProperty
 *
 * Deletes the specified property from the object.
 */
void ctr_internal_object_delete_property(ctr_object* owner, ctr_object* key, int is_method) {
	uint64_t hashKey = ctr_internal_index_hash(key);
	ctr_mapitem* head;
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
		if ((hashKey == head->hashKey) && ctr_internal_object_is_equal(head->key, key)) {
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

/**
 * InternalObjectAddProperty
 *
 * Adds a property to an object.
 */
void ctr_internal_object_add_property(ctr_object* owner, ctr_object* key, ctr_object* value, int m) {

	ctr_mapitem* new_item = malloc(sizeof(ctr_mapitem));
	ctr_mapitem* current_head = NULL;
	new_item->key = key;
	new_item->hashKey = ctr_internal_index_hash(key);
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

/**
 * InternalObjectSetProperty
 *
 * Sets a property on an object.
 */ 
void ctr_internal_object_set_property(ctr_object* owner, ctr_object* key, ctr_object* value, int is_method) {
	ctr_internal_object_delete_property(owner, key, is_method);
	ctr_internal_object_add_property(owner, key, value, is_method);
}

/**
 * InternalMemMem
 *
 * memmem implementation
 */
char* ctr_internal_memmem(char* haystack, long hlen, char* needle, long nlen, int reverse ) {
	char* cur;
	char* last;
	char* begin;
	int step = (1 - reverse * 2); /* 1 if reverse = 0, -1 if reverse = 1 */
	if (nlen == 0) return NULL;
	if (hlen == 0) return NULL;
	if (hlen < nlen) return NULL;
	if (!reverse) {
		begin = haystack;
		last = haystack + hlen - nlen + 1;
	} else {
		begin = haystack + hlen;
		last = haystack + nlen - 2;
	}
	for(cur = begin; cur!=last; cur += step) {
		if (memcmp(cur,needle,nlen) == 0) return cur;
	}
	return NULL;
}

/**
 * InternalObjectCreate
 *
 * Creates an object.
 */
ctr_object* ctr_internal_create_object(int type) {
	ctr_object* o = malloc(sizeof(ctr_object));
	o->properties = malloc(sizeof(ctr_map));
	o->methods = malloc(sizeof(ctr_map));
	o->properties->size = 0;
	o->methods->size = 0;
	o->properties->head = NULL;
	o->methods->head = NULL;
	o->info.type = type;
	o->info.sticky = 1;
	o->info.mark = 0;
	if (type==CTR_OBJECT_TYPE_OTBOOL) o->value.bvalue = 0;
	if (type==CTR_OBJECT_TYPE_OTNUMBER) o->value.nvalue = 0;
	if (type==CTR_OBJECT_TYPE_OTSTRING) {
		o->value.svalue = malloc(sizeof(ctr_string));
		o->value.svalue->value = "";
		o->value.svalue->vlen = 0;
	}
	o->gnext = NULL;
	if (ctr_first_object == NULL) {
		ctr_first_object = o;
	} else {
		o->gnext = ctr_first_object;
		ctr_first_object = o;
	}
	return o;
}

/**
 * InternalFunctionCreate
 *
 * Create a function and add this to the object as a method.
 */
void ctr_internal_create_func(ctr_object* o, ctr_object* key, ctr_object* (*func)( ctr_object*, ctr_argument* ) ) {
	ctr_object* methodObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNATFUNC);
	methodObject->value.fvalue = func;
	ctr_internal_object_add_property(o, key, methodObject, 1);
}

/**
 * InternalNumberCast
 *
 * Casts an object to a number object.
 */
ctr_object* ctr_internal_cast2number(ctr_object* o) {
	if (o->info.type == CTR_OBJECT_TYPE_OTNUMBER) return o;
	if (o->info.type == CTR_OBJECT_TYPE_OTSTRING) {
		return ctr_build_number_from_string(o->value.svalue->value, o->value.svalue->vlen);
	}
	return ctr_build_number_from_float((ctr_number)0);
}

/**
 * InternalStringCast
 *
 * Casts an object to a string object.
 */
ctr_object* ctr_internal_cast2string( ctr_object* o ) {
	int slen;
	char* s;
	switch (o->info.type) {
		case CTR_OBJECT_TYPE_OTSTRING:
			return o;
			break;
		case CTR_OBJECT_TYPE_OTNIL:
			return ctr_build_string("[Nil]", 5);
			break;
		case CTR_OBJECT_TYPE_OTBOOL:
			if (o->value.bvalue == 1) {
				return ctr_build_string("[True]", 6);
			} else if (o->value.bvalue == 0) {
				return ctr_build_string("[False]", 7);
			} else {
				return ctr_build_string("[?]", 3);
			}
			break;
		case CTR_OBJECT_TYPE_OTNUMBER:
			s = calloc(80, sizeof(char));
			CTR_CONVFP(s,o->value.nvalue);
			slen = strlen(s);
			return ctr_build_string(s, slen);
			break;
		case CTR_OBJECT_TYPE_OTBLOCK:
			return ctr_build_string("[Block]",7);
			break;
		case CTR_OBJECT_TYPE_OTOBJECT:
			return ctr_build_string("[Object]",8);
			break;
		default:
			break;
	}
	return ctr_build_string("[?]", 3);
}

/**
 * InternalBooleanCast
 *
 * Casts an object to a boolean.
 */
ctr_object* ctr_internal_cast2bool( ctr_object* o ) {
	if (o->info.type == CTR_OBJECT_TYPE_OTBOOL) return o;
	if (o->info.type == CTR_OBJECT_TYPE_OTNIL
		|| (o->info.type == CTR_OBJECT_TYPE_OTNUMBER && o->value.nvalue == 0)
		|| (o->info.type == CTR_OBJECT_TYPE_OTSTRING && o->value.svalue->vlen == 0)) return ctr_build_bool(0);
	return ctr_build_bool(1);
}

/**
 * ContextOpen
 *
 * Opens a new context to keep track of variables.
 */
void ctr_open_context() {
	ctr_object* context;
	ctr_context_id++;
	context = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	ctr_contexts[ctr_context_id] = context;
}

/**
 * ContextClose
 *
 * Closes a context.
 */
void ctr_close_context() {
	if (ctr_context_id == 0) return;
	ctr_context_id--;
}

/**
 * CTRFind
 *
 * Tries to locate a variable in the current context or one
 * of the contexts beneath.
 */
ctr_object* ctr_find(ctr_object* key) {
	int i = ctr_context_id;
	ctr_object* foundObject = NULL;
	if (CtrStdError) return CtrStdNil;
	while((i>-1 && foundObject == NULL)) {
		ctr_object* context = ctr_contexts[i];
		foundObject = ctr_internal_object_find_property(context, key, 0);
		i--;
	}
	if (foundObject == NULL) {
		ctr_internal_plugin_find(key);
		foundObject = ctr_internal_object_find_property(CtrStdWorld, key, 0);
	}
	if (foundObject == NULL) {
		char* cstr;
		CTR_2CSTR(cstr, key);
		printf("Error, key not found: [%s].\n", cstr);
		exit(1);
	}
	return foundObject;
}

/**
 * CTRFindInMy
 *
 * Tries to locate a property of an object.
 */
ctr_object* ctr_find_in_my(ctr_object* key) {
	ctr_object* context = ctr_find(ctr_build_string("me",2));
	ctr_object* foundObject = ctr_internal_object_find_property(context, key, 0);
	if (CtrStdError) return CtrStdNil;
	if (foundObject == NULL) { printf("Error, property not found: %s.\n", key->value.svalue->value); exit(1); }
	return foundObject;
}

/**
 * CTRSetBasic
 *
 * Sets a proeprty in an object (context).
 */
void ctr_set(ctr_object* key, ctr_object* object) {
	int i = ctr_context_id;
	ctr_object* context;
	ctr_object* foundObject = NULL;
	if (ctr_contexts[ctr_context_id] == CtrStdWorld) {
		ctr_internal_object_set_property(ctr_contexts[ctr_context_id], key, object, 0);
		return;
	}
	while((i>-1 && foundObject == NULL)) {
		context = ctr_contexts[i];
		foundObject = ctr_internal_object_find_property(context, key, 0);
		if (foundObject) break;
		i--;
	}
	if (!foundObject) {
		printf("Error, cannot assign, key not found: %s, forgot to use var ?\n", key->value.svalue->value); exit(1);
	}
	ctr_internal_object_set_property(context, key, object, 0);
}

/**
 * WorldInitialize
 *
 * Populate the World of Citrine.
 */
void ctr_initialize_world() {
	int i;
	srand((unsigned)time(NULL));
	for(i=0; i<16; i++) {
		CtrHashKey[i] = (rand() % 255);
	}

	ctr_first_object = NULL;

	CtrStdWorld = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	ctr_contexts[0] = CtrStdWorld;

	/* Object */
	CtrStdObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	ctr_internal_create_func(CtrStdObject, ctr_build_string("new", 3), &ctr_object_make);
	ctr_internal_create_func(CtrStdObject, ctr_build_string("equals:", 7), &ctr_object_equals);
	ctr_internal_create_func(CtrStdObject, ctr_build_string("=",1), &ctr_object_equals);
	ctr_internal_create_func(CtrStdObject, ctr_build_string("on:do:", 6), &ctr_object_on_do);
	ctr_internal_create_func(CtrStdObject, ctr_build_string("respondTo:", 10), &ctr_object_respond);
	ctr_internal_create_func(CtrStdObject, ctr_build_string("respondTo:with:", 15), &ctr_object_respond);
	ctr_internal_create_func(CtrStdObject, ctr_build_string("respondTo:with:and:", 19), &ctr_object_respond);
	ctr_internal_create_func(CtrStdObject, ctr_build_string("type", 4), &ctr_object_type);
	ctr_internal_create_func(CtrStdObject, ctr_build_string("new", 3), &ctr_object_make);
	ctr_internal_create_func(CtrStdObject, ctr_build_string("isNil", 5), &ctr_object_is_nil);
	ctr_internal_create_func(CtrStdObject, ctr_build_string("myself", 6), &ctr_object_myself);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string("Object", 6), CtrStdObject, 0);
	CtrStdObject->link = NULL;

	/* Nil */
	CtrStdNil = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNIL);

	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string("Nil", 3), CtrStdNil, 0);
	ctr_internal_create_func(CtrStdNil, ctr_build_string("isNil", 5), &ctr_nil_is_nil);
	CtrStdNil->link = CtrStdObject;
	
	/* Boolean */
	CtrStdBool = ctr_internal_create_object(CTR_OBJECT_TYPE_OTBOOL);
	ctr_internal_create_func(CtrStdBool, ctr_build_string("ifTrue:", 7), &ctr_bool_iftrue);
	ctr_internal_create_func(CtrStdBool, ctr_build_string("ifFalse:", 8), &ctr_bool_ifFalse);
	ctr_internal_create_func(CtrStdBool, ctr_build_string("break", 5), &ctr_bool_break);
	ctr_internal_create_func(CtrStdBool, ctr_build_string("continue", 8), &ctr_bool_continue);
	ctr_internal_create_func(CtrStdBool, ctr_build_string("else:", 5), &ctr_bool_ifFalse);
	ctr_internal_create_func(CtrStdBool, ctr_build_string("not", 3), &ctr_bool_not);
	ctr_internal_create_func(CtrStdBool, ctr_build_string("and:", 4), &ctr_bool_and);
	ctr_internal_create_func(CtrStdBool, ctr_build_string("nor:", 4), &ctr_bool_nor);
	ctr_internal_create_func(CtrStdBool, ctr_build_string("or:", 3), &ctr_bool_or);
	ctr_internal_create_func(CtrStdBool, ctr_build_string("xor:", 4), &ctr_bool_xor);
	ctr_internal_create_func(CtrStdBool, ctr_build_string("=",1),&ctr_bool_eq);
	ctr_internal_create_func(CtrStdBool, ctr_build_string("≠",3),&ctr_bool_neq);
	ctr_internal_create_func(CtrStdBool, ctr_build_string("toNumber", 8), &ctr_bool_to_number);
	ctr_internal_create_func(CtrStdBool, ctr_build_string("toString", 8), &ctr_bool_to_string);
	ctr_internal_create_func(CtrStdBool, ctr_build_string("flip", 4), &ctr_bool_flip);
	ctr_internal_create_func(CtrStdBool, ctr_build_string("either:or:", 10), &ctr_bool_either_or);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string("Boolean", 7), CtrStdBool, 0);
	CtrStdBool->link = CtrStdObject;

	/* Number */
	CtrStdNumber = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNUMBER);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("to:by:do:", 9), &ctr_number_to_by_do);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("+", 1), &ctr_number_add);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("add:",4), &ctr_number_inc);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("-",1), &ctr_number_minus);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("subtract:",9), &ctr_number_dec);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("*",1),&ctr_number_multiply);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("times:",6),&ctr_number_times);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("multiplyBy:",11),&ctr_number_mul);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("/",1), &ctr_number_divide);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("divideBy:",9),&ctr_number_div);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string(">",1),&ctr_number_higherThan);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("≥",3),&ctr_number_higherEqThan);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("<",1),&ctr_number_lowerThan);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("≤",3),&ctr_number_lowerEqThan);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("=",1),&ctr_number_eq);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("≠",3),&ctr_number_neq);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("%",1),&ctr_number_modulo);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("factorial",9),&ctr_number_factorial);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("floor",5),&ctr_number_floor);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("ceiling",7),&ctr_number_ceil);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("round",5),&ctr_number_round);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("absolute",8),&ctr_number_abs);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("sine",4),&ctr_number_sin);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("cosine",6),&ctr_number_cos);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("exponent",8),&ctr_number_exp);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("squareRoot",10),&ctr_number_sqrt);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("tangent",7),&ctr_number_tan);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("arctangent",10),&ctr_number_atan);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("logarithm",9),&ctr_number_log);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("toPowerOf:",10),&ctr_number_pow);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("min:",4),&ctr_number_min);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("max:",4),&ctr_number_max);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("odd",3),&ctr_number_odd);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("even",4),&ctr_number_even);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("positive",8),&ctr_number_positive);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("negative",8),&ctr_number_negative);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("toString", 8), &ctr_number_to_string);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("toBoolean", 9), &ctr_number_to_boolean);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string("between:and:",12),&ctr_number_between);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string("Number", 6), CtrStdNumber, 0);
	CtrStdNumber->link = CtrStdObject;

	/* String */
	CtrStdString = ctr_internal_create_object(CTR_OBJECT_TYPE_OTSTRING);
	ctr_internal_create_func(CtrStdString, ctr_build_string("bytes", 5), &ctr_string_bytes);
	ctr_internal_create_func(CtrStdString, ctr_build_string("length", 6), &ctr_string_length);
	ctr_internal_create_func(CtrStdString, ctr_build_string("from:to:", 8), &ctr_string_fromto);
	ctr_internal_create_func(CtrStdString, ctr_build_string("from:length:", 12), &ctr_string_from_length);
	ctr_internal_create_func(CtrStdString, ctr_build_string("+", 1), &ctr_string_concat);
	ctr_internal_create_func(CtrStdString, ctr_build_string("=", 1), &ctr_string_eq);
	ctr_internal_create_func(CtrStdString, ctr_build_string("≠", 3), &ctr_string_neq);
	ctr_internal_create_func(CtrStdString, ctr_build_string("trim", 4), &ctr_string_trim);
	ctr_internal_create_func(CtrStdString, ctr_build_string("leftTrim", 8), &ctr_string_ltrim);
	ctr_internal_create_func(CtrStdString, ctr_build_string("rightTrim", 9), &ctr_string_rtrim);
	ctr_internal_create_func(CtrStdString, ctr_build_string("htmlEscape", 10), &ctr_string_html_escape);
	ctr_internal_create_func(CtrStdString, ctr_build_string("at:", 3), &ctr_string_at);
	ctr_internal_create_func(CtrStdString, ctr_build_string("byteAt:", 7), &ctr_string_byte_at);
	ctr_internal_create_func(CtrStdString, ctr_build_string("indexOf:", 8), &ctr_string_index_of);
	ctr_internal_create_func(CtrStdString, ctr_build_string("lastIndexOf:", 12), &ctr_string_last_index_of);
	ctr_internal_create_func(CtrStdString, ctr_build_string("replace:with:", 13), &ctr_string_replace_with);
	ctr_internal_create_func(CtrStdString, ctr_build_string("split:", 6), &ctr_string_split);
	ctr_internal_create_func(CtrStdString, ctr_build_string("asciiUpperCase", 14), &ctr_string_to_upper);
	ctr_internal_create_func(CtrStdString, ctr_build_string("asciiLowerCase", 14), &ctr_string_to_lower);
	ctr_internal_create_func(CtrStdString, ctr_build_string("asciiUpperCase1st", 17), &ctr_string_to_upper1st);
	ctr_internal_create_func(CtrStdString, ctr_build_string("asciiLowerCase1st", 17), &ctr_string_to_lower1st);
	ctr_internal_create_func(CtrStdString, ctr_build_string("skip:", 5), &ctr_string_skip);
	ctr_internal_create_func(CtrStdString, ctr_build_string("append:", 7), &ctr_string_append);
	ctr_internal_create_func(CtrStdString, ctr_build_string("toNumber", 8), &ctr_string_to_number);
	ctr_internal_create_func(CtrStdString, ctr_build_string("toBoolean", 9), &ctr_string_to_boolean);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string("String", 6), CtrStdString, 0);
	CtrStdString->link = CtrStdObject;

	/* Block */
	CtrStdBlock = ctr_internal_create_object(CTR_OBJECT_TYPE_OTBLOCK);
	ctr_internal_create_func(CtrStdBlock, ctr_build_string("run", 3), &ctr_block_runIt);
	ctr_internal_create_func(CtrStdBlock, ctr_build_string("applyTo:", 8), &ctr_block_runIt);
	ctr_internal_create_func(CtrStdBlock, ctr_build_string("applyTo:and:", 12), &ctr_block_runIt);
	ctr_internal_create_func(CtrStdBlock, ctr_build_string("applyTo:and:and:", 16), &ctr_block_runIt);
	ctr_internal_create_func(CtrStdBlock, ctr_build_string("set:value:", 10), &ctr_block_set);
	ctr_internal_create_func(CtrStdBlock, ctr_build_string("error:", 6), &ctr_block_error);
	ctr_internal_create_func(CtrStdBlock, ctr_build_string("catch:", 6), &ctr_block_catch);
	ctr_internal_create_func(CtrStdBlock, ctr_build_string("whileTrue:", 10), &ctr_block_while_true);
	ctr_internal_create_func(CtrStdBlock, ctr_build_string("whileFalse:", 11), &ctr_block_while_false);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string("CodeBlock", 9), CtrStdBlock, 0);
	CtrStdBlock->link = CtrStdObject;

	/* Array */
	CtrStdArray = ctr_array_new(CtrStdObject, NULL);
	ctr_internal_create_func(CtrStdArray, ctr_build_string("new", 3), &ctr_array_new);
	ctr_internal_create_func(CtrStdArray, ctr_build_string("←", 3), &ctr_array_new_and_push);
	ctr_internal_create_func(CtrStdArray, ctr_build_string("push:", 5), &ctr_array_push);
	ctr_internal_create_func(CtrStdArray, ctr_build_string(";", 1), &ctr_array_push);
	ctr_internal_create_func(CtrStdArray, ctr_build_string("unshift:", 8), &ctr_array_unshift);
	ctr_internal_create_func(CtrStdArray, ctr_build_string("shift", 5), &ctr_array_shift);
	ctr_internal_create_func(CtrStdArray, ctr_build_string("count", 5), &ctr_array_count);
	ctr_internal_create_func(CtrStdArray, ctr_build_string("join:", 5), &ctr_array_join);
	ctr_internal_create_func(CtrStdArray, ctr_build_string("pop", 3), &ctr_array_pop);
	ctr_internal_create_func(CtrStdArray, ctr_build_string("at:", 3), &ctr_array_get);
	ctr_internal_create_func(CtrStdArray, ctr_build_string("@", 1), &ctr_array_get);
	ctr_internal_create_func(CtrStdArray, ctr_build_string("sort:", 5), &ctr_array_sort);
	ctr_internal_create_func(CtrStdArray, ctr_build_string("put:at:", 7), &ctr_array_put);
	ctr_internal_create_func(CtrStdArray, ctr_build_string("from:length:", 12), &ctr_array_from_to);
	ctr_internal_create_func(CtrStdArray, ctr_build_string("+", 1), &ctr_array_add);
	ctr_internal_create_func(CtrStdArray, ctr_build_string("map:", 4), &ctr_array_map);
	ctr_internal_create_func(CtrStdArray, ctr_build_string("each:", 5), &ctr_array_map);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string("Array", 5), CtrStdArray, 0);
	CtrStdArray->link = CtrStdObject;

	/* Map */
	CtrStdMap = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	ctr_internal_create_func(CtrStdMap, ctr_build_string("new", 3), &ctr_map_new);
	ctr_internal_create_func(CtrStdMap, ctr_build_string("put:at:", 7), &ctr_map_put);
	ctr_internal_create_func(CtrStdMap, ctr_build_string("at:", 3), &ctr_map_get);
	ctr_internal_create_func(CtrStdMap, ctr_build_string("@", 1), &ctr_map_get);
	ctr_internal_create_func(CtrStdMap, ctr_build_string("count", 5), &ctr_map_count);
	ctr_internal_create_func(CtrStdMap, ctr_build_string("each:", 5), &ctr_map_each);
	ctr_internal_create_func(CtrStdMap, ctr_build_string("map:", 4), &ctr_map_each);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string("Map", 3), CtrStdMap, 0);
	CtrStdMap->link = CtrStdObject;

	/* Console */
	CtrStdConsole = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	ctr_internal_create_func(CtrStdConsole, ctr_build_string("write:", 6), &ctr_console_write);
	ctr_internal_create_func(CtrStdConsole, ctr_build_string(">", 1), &ctr_console_write);
	ctr_internal_create_func(CtrStdConsole, ctr_build_string("brk", 3), &ctr_console_brk);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string("Pen", 3), CtrStdConsole, 0);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string("?", 1), CtrStdConsole, 0);
	CtrStdConsole->link = CtrStdObject;
	
	/* File */
	CtrStdFile = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	CtrStdFile->value.rvalue = NULL;
	ctr_internal_create_func(CtrStdFile, ctr_build_string("new:", 4), &ctr_file_new);
	ctr_internal_create_func(CtrStdFile, ctr_build_string("path", 4), &ctr_file_path);
	ctr_internal_create_func(CtrStdFile, ctr_build_string("read", 4), &ctr_file_read);
	ctr_internal_create_func(CtrStdFile, ctr_build_string("write:", 6), &ctr_file_write);
	ctr_internal_create_func(CtrStdFile, ctr_build_string("append:", 7), &ctr_file_append);
	ctr_internal_create_func(CtrStdFile, ctr_build_string("exists", 6), &ctr_file_exists);
	ctr_internal_create_func(CtrStdFile, ctr_build_string("size", 4), &ctr_file_size);
	ctr_internal_create_func(CtrStdFile, ctr_build_string("delete", 6), &ctr_file_delete);
	ctr_internal_create_func(CtrStdFile, ctr_build_string("include", 7), &ctr_file_include);
	ctr_internal_create_func(CtrStdFile, ctr_build_string("go", 2), &ctr_file_include_ast);
	ctr_internal_create_func(CtrStdFile, ctr_build_string("open:", 5), &ctr_file_open);
	ctr_internal_create_func(CtrStdFile, ctr_build_string("close", 5), &ctr_file_close);
	ctr_internal_create_func(CtrStdFile, ctr_build_string("readBytes:", 10), &ctr_file_read_bytes);
	ctr_internal_create_func(CtrStdFile, ctr_build_string("writeBytes:", 11), &ctr_file_write_bytes);
	ctr_internal_create_func(CtrStdFile, ctr_build_string("seek:", 5), &ctr_file_seek);
	ctr_internal_create_func(CtrStdFile, ctr_build_string("rewind", 6), &ctr_file_seek_rewind);
	ctr_internal_create_func(CtrStdFile, ctr_build_string("end", 3), &ctr_file_seek_end);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string("File", 4), CtrStdFile, 0);
	CtrStdFile->link = CtrStdObject;

	/* Command */
	CtrStdCommand = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	ctr_internal_create_func(CtrStdCommand, ctr_build_string("argument:", 9), &ctr_command_argument);
	ctr_internal_create_func(CtrStdCommand, ctr_build_string("argCount", 8), &ctr_command_num_of_args);
	ctr_internal_create_func(CtrStdCommand, ctr_build_string("env:", 4), &ctr_command_get_env);
	ctr_internal_create_func(CtrStdCommand, ctr_build_string("env:val:", 8), &ctr_command_set_env);
	ctr_internal_create_func(CtrStdCommand, ctr_build_string("askQuestion", 11), &ctr_command_question);
	ctr_internal_create_func(CtrStdCommand, ctr_build_string("exit", 4), &ctr_command_exit);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string("Command", 7), CtrStdCommand, 0);
	CtrStdCommand->link = CtrStdObject;

	/* Clock */
	CtrStdClock = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	ctr_internal_create_func(CtrStdClock, ctr_build_string("wait:", 5), &ctr_clock_wait);
	ctr_internal_create_func(CtrStdClock, ctr_build_string("time", 4), &ctr_clock_time);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string("Clock", 5), CtrStdClock, 0);
	CtrStdClock->link = CtrStdObject;

	/* Dice */
	CtrStdDice = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	ctr_internal_create_func(CtrStdDice, ctr_build_string("roll", 4), &ctr_dice_throw);
	ctr_internal_create_func(CtrStdDice, ctr_build_string("rollWithSides:", 14), &ctr_dice_sides);
	ctr_internal_create_func(CtrStdDice, ctr_build_string("rawRandomNumber", 15), &ctr_dice_rand);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string("Dice", 4), CtrStdDice, 0);
	CtrStdDice->link = CtrStdObject;

	/* Shell */
	CtrStdShell = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	ctr_internal_create_func(CtrStdShell, ctr_build_string("respondTo:", 10), &ctr_shell_respond_to);
	ctr_internal_create_func(CtrStdShell, ctr_build_string("respondTo:with:", 15), &ctr_shell_respond_to_with);
	ctr_internal_create_func(CtrStdShell, ctr_build_string("call:", 5), &ctr_shell_call);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string("Shell", 5), CtrStdShell, 0);
	CtrStdShell->link = CtrStdObject;

	/* Broom */
	CtrStdGC = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	ctr_internal_create_func(CtrStdGC, ctr_build_string("sweep", 5), &ctr_gc_collect);
	ctr_internal_create_func(CtrStdGC, ctr_build_string("dust", 4), &ctr_gc_dust);
	ctr_internal_create_func(CtrStdGC, ctr_build_string("objectCount", 11), &ctr_gc_object_count);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string("Broom", 5), CtrStdGC, 0);
	CtrStdGC->link = CtrStdObject;
	
	/* Other objects */
	CtrStdBreak = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	CtrStdContinue = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
}

/**
 * CTRMessageSend
 *
 * Sends a message to a receiver object.
 */
ctr_object* ctr_send_message(ctr_object* receiverObject, char* message, long vlen, ctr_argument* argumentList) {
	char toParent = 0;
	ctr_object* me;
	ctr_object* methodObject;
	ctr_object* searchObject;
	ctr_argument* argCounter;
	ctr_argument* mesgArgument;
	ctr_object* result;
	ctr_object* (*funct)(ctr_object* receiverObject, ctr_argument* argumentList);
	int argCount;
	if (CtrStdError != NULL) return NULL; /* Error mode, ignore subsequent messages until resolved. */
	methodObject = NULL;
	searchObject = receiverObject;
	if (vlen > 1 && message[0] == '`') {
		me = ctr_internal_object_find_property(ctr_contexts[ctr_context_id], ctr_build_string_from_cstring("me\0"), 0);
		if (searchObject == me) {
			toParent = 1;
			message = message + 1;
			vlen--;
		}
	}
	while(!methodObject) {
		methodObject = ctr_internal_object_find_property(searchObject, ctr_build_string(message, vlen), 1);
		if (methodObject && toParent) { toParent = 0; methodObject = NULL; }
		if (methodObject) break;
		if (!searchObject->link) break;
		searchObject = searchObject->link;
	}
	if (!methodObject) {
		argCounter = argumentList;
		argCount = 0;
		while(argCounter->next && argCount < 4) {
			argCounter = argCounter->next;
			argCount ++;
		}
		mesgArgument = CTR_CREATE_ARGUMENT();
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
	if (methodObject->info.type == CTR_OBJECT_TYPE_OTNATFUNC) {
		funct = methodObject->value.fvalue;
		result = funct(receiverObject, argumentList);
	}
	if (methodObject->info.type == CTR_OBJECT_TYPE_OTBLOCK) {
		result = ctr_block_run(methodObject, argumentList, receiverObject);
	}	
	return result;
}


/**
 * CTRValueAssignment
 *
 * Assigns a value to a variable in the current context.
 */
ctr_object* ctr_assign_value(ctr_object* key, ctr_object* o) {
	ctr_object* object = NULL;
	if (CtrStdError) return CtrStdNil;
	key->info.sticky = 0;
	if (o->info.type == CTR_OBJECT_TYPE_OTOBJECT || o->info.type == CTR_OBJECT_TYPE_OTMISC || o->info.type == CTR_OBJECT_TYPE_OTARRAY || o->info.type == CTR_OBJECT_TYPE_OTNIL) {
		ctr_set(key, o);
	} else {
		object = ctr_internal_create_object(o->info.type);
		object->properties = o->properties;
		object->methods = o->methods;
		object->link = o->link;
		object->info.sticky = 0;
		ctr_set(key, object);
	}

	/* depending on type, copy specific value */
	switch (o->info.type) {
		case CTR_OBJECT_TYPE_OTBOOL:
			object->value.bvalue = o->value.bvalue;
			break;
		case CTR_OBJECT_TYPE_OTNUMBER:
			object->value.nvalue = o->value.nvalue;
			break;
		case CTR_OBJECT_TYPE_OTSTRING:
			object->value.svalue = malloc(sizeof(ctr_string));
			object->value.svalue->value = malloc(sizeof(char)*o->value.svalue->vlen);
			memcpy(object->value.svalue->value, o->value.svalue->value,o->value.svalue->vlen);
			object->value.svalue->vlen = o->value.svalue->vlen;
			break;
		case CTR_OBJECT_TYPE_OTBLOCK:
			object->value.block = o->value.block;
			break;
		default:
			break;
	}
	return object;
}


/**
 * CTRAssignValueObject
 *
 * Assigns a value to a property of an object. 
 */
ctr_object* ctr_assign_value_to_my(ctr_object* key, ctr_object* o) {
	ctr_object* object = NULL;
	ctr_object* my = ctr_find(ctr_build_string("me", 2));
	if (CtrStdError) return CtrStdNil;
	key->info.sticky = 0;
	if (o->info.type == CTR_OBJECT_TYPE_OTOBJECT || o->info.type == CTR_OBJECT_TYPE_OTMISC || o->info.type == CTR_OBJECT_TYPE_OTARRAY || o->info.type == CTR_OBJECT_TYPE_OTNIL) {
		ctr_internal_object_set_property(my, key, o, 0);
	} else {
		object = ctr_internal_create_object(o->info.type);
		object->properties = o->properties;
		object->methods = o->methods;
		object->link = o->link;
		object->info.sticky = 0;
		ctr_internal_object_set_property(my, key, object, 0);
	}

	/* depending on type, copy specific value */
	switch (o->info.type) {
		case CTR_OBJECT_TYPE_OTBOOL:
			object->value.bvalue = o->value.bvalue;
			break;
		case CTR_OBJECT_TYPE_OTNUMBER:
			object->value.nvalue = o->value.nvalue;
			break;
		case CTR_OBJECT_TYPE_OTSTRING:
			object->value.svalue = malloc(sizeof(ctr_string));
			object->value.svalue->value = malloc(sizeof(char)*o->value.svalue->vlen);
			memcpy(object->value.svalue->value, o->value.svalue->value,o->value.svalue->vlen);
			object->value.svalue->vlen = o->value.svalue->vlen;
			break;
		case CTR_OBJECT_TYPE_OTBLOCK:
			object->value.block = o->value.block;
			break;
		default:
			break;

	}
	return object;
}

/**
 * CTRAssignValueObjectLocal
 *
 * Assigns a value to a local of an object. 
 */
ctr_object* ctr_assign_value_to_local(ctr_object* key, ctr_object* o) {
	ctr_object* object = NULL;
	ctr_object* context;
	if (CtrStdError) return CtrStdNil;
	context = ctr_contexts[ctr_context_id];
	key->info.sticky = 0;
	if (o->info.type == CTR_OBJECT_TYPE_OTOBJECT || o->info.type == CTR_OBJECT_TYPE_OTMISC || o->info.type == CTR_OBJECT_TYPE_OTARRAY || o->info.type == CTR_OBJECT_TYPE_OTNIL) {
		ctr_internal_object_set_property(context, key, o, 0);
	} else {
		object = ctr_internal_create_object(o->info.type);
		object->properties = o->properties;
		object->methods = o->methods;
		object->link = o->link;
		object->info.sticky = 0;
		ctr_internal_object_set_property(context, key, object, 0);
	}

	/* depending on type, copy specific value */
	switch (o->info.type) {
		case CTR_OBJECT_TYPE_OTBOOL:
			object->value.bvalue = o->value.bvalue;
			break;
		case CTR_OBJECT_TYPE_OTNUMBER:
			object->value.nvalue = o->value.nvalue;
			break;
		case CTR_OBJECT_TYPE_OTSTRING:
			object->value.svalue = malloc(sizeof(ctr_string));
			object->value.svalue->value = malloc(sizeof(char)*o->value.svalue->vlen);
			memcpy(object->value.svalue->value, o->value.svalue->value,o->value.svalue->vlen);
			object->value.svalue->vlen = o->value.svalue->vlen;
			break;
		case CTR_OBJECT_TYPE_OTBLOCK:
			object->value.block = o->value.block;
			break;
		default:
			break;
	}
	return object;
}
