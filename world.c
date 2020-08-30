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

ctr_object* CtrStdWorld;
ctr_object* CtrStdObject;
ctr_object* CtrStdBlock;
ctr_object* CtrStdString;
ctr_object* CtrStdNumber;
ctr_object* CtrStdBool;
ctr_object* CtrStdConsole;
ctr_object* CtrStdNil;
ctr_object* CtrStdGC;
ctr_object* CtrStdMap;
ctr_object* CtrStdArray;
ctr_object* CtrStdFile;
ctr_object* CtrStdSystem;
ctr_object* CtrStdCommand;
ctr_object* CtrStdClock;
ctr_object* CtrStdFlow;
ctr_object* CtrStdBreak;
ctr_object* CtrStdContinue;
ctr_object* CtrStdExit;
ctr_object* ctr_first_object;
ctr_object* CtrStdBoolTrue;
ctr_object* CtrStdBoolFalse;
char CtrHashKey[16];

ctr_object* ctr_contexts[301];
int ctr_context_id;

char ctr_deserialize_mode;

/**
 * ?internal
 *
 * ReadFile
 *
 * Reads in an entire file.
 */
char* ctr_internal_readf(char* file_name, uint64_t* total_size) {
   char* prg;
   char ch;
   int prev;
   uint64_t size;
   uint64_t real_size;
   FILE* fp;
   fp = fopen(file_name,"r");
   if( fp == NULL ) {
      fprintf(stderr, CTR_ERR_FOPEN );
      exit(1);
   }
   prev = ftell(fp);
   fseek(fp,0L,SEEK_END);
   size = ftell(fp);
   fseek(fp,prev,SEEK_SET);
   real_size = (size+4)*sizeof(char);
   prg = ctr_heap_allocate(real_size); /* add 4 bytes, 3 for optional closing sequence verbatim mode and one lucky byte! */
   ctr_program_length=0;
   while( ( ch = fgetc(fp) ) != EOF ) prg[ctr_program_length++]=ch;
   if ( ctr_program_length != size ) {
	fprintf(stderr, CTR_ERR_SEEK );
	exit(1);
   }
   fclose(fp);
   *total_size = (uint64_t) real_size;
   return prg;
}

/**
 * ?internal
 *
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
 * ?internal
 *
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
 * ?internal
 *
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
 * ?internal
 *
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
			ctr_heap_free( head );
			return;
		}
		head = head->next;
	}
	return;
}

/**
 * ?internal
 *
 * InternalObjectAddProperty
 *
 * Adds a property to an object.
 */
void ctr_internal_object_add_property(ctr_object* owner, ctr_object* key, ctr_object* value, int m) {
	ctr_mapitem* new_item = ctr_heap_allocate(sizeof(ctr_mapitem));
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
 * ?internal
 *
 * InternalObjectSetProperty
 *
 * Sets a property on an object.
 */
void ctr_internal_object_set_property(ctr_object* owner, ctr_object* key, ctr_object* value, int is_method) {
	ctr_internal_object_delete_property(owner, key, is_method);
	ctr_internal_object_add_property(owner, key, value, is_method);
}

/**
 * ?internal
 *
 * InternalMemMem
 *
 * memmem implementation because this not available on every system.
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
		begin = haystack + hlen - nlen;
		last = haystack - 1;
	}
	for(cur = begin; cur!=last; cur += step) {
		if (memcmp(cur,needle,nlen) == 0) return cur;
	}
	return NULL;
}

/**
 * ?internal
 *
 * InternalObjectCreate
 *
 * Creates an object.
 */
ctr_object* ctr_internal_create_object(int type) {
	ctr_object* o;
	o = ctr_heap_allocate(sizeof(ctr_object));
	o->properties = ctr_heap_allocate(sizeof(ctr_map));
	o->methods = ctr_heap_allocate(sizeof(ctr_map));
	o->properties->size = 0;
	o->methods->size = 0;
	o->properties->head = NULL;
	o->methods->head = NULL;
	o->info.type = type;
	o->info.sticky = 0;
	o->info.mark = 0;
	o->info.selfbind = 0;
	if (type==CTR_OBJECT_TYPE_OTBOOL) o->value.bvalue = 0;
	if (type==CTR_OBJECT_TYPE_OTNUMBER) o->value.nvalue = 0;
	if (type==CTR_OBJECT_TYPE_OTSTRING) {
		o->value.svalue = ctr_heap_allocate(sizeof(ctr_string));
		o->value.svalue->value = "";
		o->value.svalue->vlen = 0;
	}
	if (type==CTR_OBJECT_TYPE_OTBLOCK) {
		o->value.block = NULL;
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
 * ?internal
 *
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
 * ?internal
 *
 * InternalNumberCast
 *
 * Casts an object to a number object.
 */
ctr_object* ctr_internal_cast2number(ctr_object* o) {
	if ( o->info.type == CTR_OBJECT_TYPE_OTNUMBER ) return o;
	ctr_argument* a = ctr_heap_allocate( sizeof( ctr_argument ) );
	a->object = CtrStdNil;
	ctr_object* numObject = ctr_send_message( o, CTR_DICT_TONUMBER, strlen(CTR_DICT_TONUMBER), a );
	ctr_heap_free(a);
	if ( numObject->info.type != CTR_OBJECT_TYPE_OTNUMBER ) {
		CtrStdFlow = ctr_error( CTR_ERR_RNUM, 0 );
		return ctr_build_number_from_float((ctr_number)0);
	}
	return numObject;
}

/**
 * ?internal
 *
 * InternalStringCast
 *
 * Casts an object to a string object.
 */
ctr_object* ctr_internal_cast2string( ctr_object* o ) {
	if ( o->info.type == CTR_OBJECT_TYPE_OTSTRING ) return o;
	ctr_argument* a = ctr_heap_allocate( sizeof( ctr_argument ) );
	a->object = CtrStdNil;
	ctr_object* stringObject = ctr_send_message( o, CTR_DICT_TOSTRING, strlen(CTR_DICT_TOSTRING), a );
	ctr_heap_free(a);
	if ( stringObject->info.type != CTR_OBJECT_TYPE_OTSTRING ) {
		CtrStdFlow = ctr_error( CTR_ERR_RSTR, 0 );
		return ctr_build_string_from_cstring( "?" );
	}
	return stringObject;
}

/**
 * ?internal
 *
 * InternalBooleanCast
 *
 * Casts an object to a boolean.
 */
ctr_object* ctr_internal_cast2bool( ctr_object* o ) {
	if (o->info.type == CTR_OBJECT_TYPE_OTBOOL) return o;
	ctr_argument* a = ctr_heap_allocate( sizeof( ctr_argument ) );
	a->object = CtrStdNil;
	ctr_object* boolObject = ctr_send_message( o, CTR_DICT_TOBOOL, strlen(CTR_DICT_TOBOOL), a );
	ctr_heap_free(a);
	if ( boolObject->info.type != CTR_OBJECT_TYPE_OTBOOL ) {
		CtrStdFlow = ctr_error( CTR_ERR_RBOOL, 0 );
		return ctr_build_bool(0);
	}
	return boolObject;
}

/**
 * ?internal
 *
 * ContextOpen
 *
 * Opens a new context to keep track of variables.
 */
void ctr_open_context() {
	ctr_object* context;
	if (ctr_context_id >= 299) {
		CtrStdFlow = ctr_error( CTR_ERR_NESTING, 0 );
	}
	context = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	context->info.sticky = 1;
	ctr_contexts[++ctr_context_id] = context;
}

/**
 * ?internal
 *
 * ContextClose
 *
 * Closes a context.
 */
void ctr_close_context() {
	ctr_contexts[ctr_context_id]->info.sticky = 0;
	if (ctr_context_id == 0) return;
	ctr_context_id--;
}

/**
 * ?internal
 *
 * CTRFind
 *
 * Tries to locate a variable in the current context or one
 * of the contexts beneath.
 */
ctr_object* ctr_find(ctr_object* key) {
	int i = ctr_context_id;
	ctr_object* foundObject = NULL;
	if (CtrStdFlow) return CtrStdNil;
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
		char* key_name;
		char* message;
		char* full_message;
		int message_size;
		message = CTR_ERR_KNF;
		message_size = ((strlen(message))+key->value.svalue->vlen);
		full_message = ctr_heap_allocate( message_size * sizeof( char ) );
		key_name = ctr_heap_allocate_cstring( key );
		memcpy(full_message, message, strlen(message));
		memcpy(full_message + strlen(message), key_name, key->value.svalue->vlen);
		CtrStdFlow = ctr_build_string(full_message, message_size);
		CtrStdFlow->info.sticky = 1;
		ctr_heap_free( full_message );
		ctr_heap_free( key_name );
		return CtrStdNil;
	}
	return foundObject;
}

/**
 * ?internal
 *
 * CTRFindInMy
 *
 * Tries to locate a property of an object.
 */
ctr_object* ctr_find_in_my(ctr_object* key) {
	ctr_object* context = ctr_find(ctr_build_string_from_cstring( ctr_clex_keyword_me_icon ) );
	ctr_object* foundObject = ctr_internal_object_find_property(context, key, 0);
	if (CtrStdFlow) return CtrStdNil;
	if (foundObject == NULL) {
		char* key_name;
		char* message;
		char* full_message;
		int message_size;
		message = CTR_ERR_KNF;
		message_size = ((strlen(message))+key->value.svalue->vlen);
		full_message = ctr_heap_allocate( message_size * sizeof( char ) );
		key_name = ctr_heap_allocate_cstring( key );
		memcpy(full_message, message, strlen(message));
		memcpy(full_message + strlen(message), key_name, key->value.svalue->vlen);
		CtrStdFlow = ctr_build_string(full_message, message_size);
		CtrStdFlow->info.sticky = 1;
		ctr_heap_free( full_message );
		ctr_heap_free( key_name );
		return CtrStdNil;
	}
	return foundObject;
}

/**
 * ?internal
 *
 * CTRSetBasic
 *
 * Sets a proeprty in an object (context).
 */
void ctr_set(ctr_object* key, ctr_object* object) {
	int i = ctr_context_id;
	ctr_object* context;
	ctr_object* foundObject = NULL;
	while((i>-1 && foundObject == NULL)) {
		context = ctr_contexts[i];
		foundObject = ctr_internal_object_find_property(context, key, 0);
		if (foundObject) break;
		i--;
	}
	if (!foundObject) {
		char* key_name;
		char* message;
		char* full_message;
		int message_size;
		message = CTR_ERR_ASSIGN;
		message_size = ((strlen(message))+key->value.svalue->vlen);
		full_message = ctr_heap_allocate(message_size*sizeof(char));
		key_name = ctr_heap_allocate_cstring( key );
		memcpy(full_message, message, strlen(message));
		memcpy(full_message + strlen(message), key_name, key->value.svalue->vlen);
		CtrStdFlow = ctr_build_string(full_message, message_size);
		CtrStdFlow->info.sticky = 1;
		ctr_heap_free( full_message );
		ctr_heap_free( key_name );
		return;
	}
	ctr_internal_object_set_property(context, key, object, 0);
}

/**
 * ?internal
 *
 * WorldInitialize
 *
 * Populate the World of Citrine.
 */
void ctr_initialize_world() {
	int i;
	srand((unsigned)time(NULL));
	for(i=0; i<16; i++) {
		CtrHashKey[i] = (int) (rand() % 256);
	}

	ctr_first_object = NULL;
	CtrStdWorld = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	CtrStdWorld->info.sticky = 1;
	ctr_contexts[0] = CtrStdWorld;
	
	/* Object */
	CtrStdObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_object_make );
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_EQUALS ), &ctr_object_equals );
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_SYMBOL_EQUALS ), &ctr_object_equals );
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_ONDO ), &ctr_object_on_do );
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_RESPOND_TO ), &ctr_object_respond );
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_RESPOND_TO_AND ), &ctr_object_respond_and );
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_RESPOND_TO_AND_AND ), &ctr_object_respond_and_and);
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_RESPOND_TO_AND_AND_AND ), &ctr_object_respond_and_and_and);
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_TYPE ), &ctr_object_type );
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_ISNIL ), &ctr_object_is_nil );
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_MYSELF ), &ctr_object_myself );
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_DO ), &ctr_object_do );
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_DONE ), &ctr_object_done );
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_IFFALSE ), &ctr_object_if_false );
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_IFTRUE ), &ctr_object_if_true );
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_MESSAGEARGS), &ctr_object_message );
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_TOSTRING ), &ctr_object_to_string );
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_TONUMBER ), &ctr_object_to_number );
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_TOBOOL ), &ctr_object_to_boolean );
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_CASE_DO ), &ctr_object_case_do );
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_LEARN ), &ctr_object_learn_meaning );
	ctr_internal_create_func( CtrStdObject, ctr_build_string_from_cstring( CTR_DICT_CODE ), &ctr_object_to_code );
	ctr_internal_object_add_property( CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_OBJECT ), CtrStdObject, 0 );
	CtrStdObject->link = NULL;
	CtrStdObject->info.sticky = 1;

	/* Nil */
	CtrStdNil = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNIL);
	ctr_internal_object_add_property( CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_NIL ), CtrStdNil, 0 );
	ctr_internal_create_func( CtrStdNil, ctr_build_string_from_cstring( CTR_DICT_ISNIL ), &ctr_nil_is_nil );
	ctr_internal_create_func( CtrStdNil, ctr_build_string_from_cstring( CTR_DICT_TOSTRING ), &ctr_nil_to_string );
	ctr_internal_create_func( CtrStdNil, ctr_build_string_from_cstring( CTR_DICT_TONUMBER ), &ctr_nil_to_number );
	ctr_internal_create_func( CtrStdNil, ctr_build_string_from_cstring( CTR_DICT_TOBOOL ), &ctr_nil_to_boolean );
	ctr_internal_create_func( CtrStdNil, ctr_build_string_from_cstring( CTR_DICT_CODE ), &ctr_nil_to_string );
	CtrStdNil->link = CtrStdObject;
	CtrStdNil->info.sticky = 1;

	/* Boolean */
	CtrStdBool = ctr_internal_create_object(CTR_OBJECT_TYPE_OTBOOL);
	ctr_internal_create_func( CtrStdBool, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_bool_new );
	ctr_internal_create_func( CtrStdBool, ctr_build_string_from_cstring( CTR_DICT_IFTRUE ), &ctr_bool_if_true );
	ctr_internal_create_func( CtrStdBool, ctr_build_string_from_cstring( CTR_DICT_IFFALSE ), &ctr_bool_if_false );
	ctr_internal_create_func( CtrStdBool, ctr_build_string_from_cstring( CTR_DICT_BREAK ), &ctr_bool_break );
	ctr_internal_create_func( CtrStdBool, ctr_build_string_from_cstring( CTR_DICT_CONTINUE ), &ctr_bool_continue );
	ctr_internal_create_func( CtrStdBool, ctr_build_string_from_cstring( CTR_DICT_ELSE ), &ctr_bool_if_false );
	ctr_internal_create_func( CtrStdBool, ctr_build_string_from_cstring( CTR_DICT_NOT ), &ctr_bool_not );
	ctr_internal_create_func( CtrStdBool, ctr_build_string_from_cstring( CTR_DICT_AND ), &ctr_bool_and );
	ctr_internal_create_func( CtrStdBool, ctr_build_string_from_cstring( CTR_DICT_NOR ), &ctr_bool_nor );
	ctr_internal_create_func( CtrStdBool, ctr_build_string_from_cstring( CTR_DICT_OR ), &ctr_bool_or );
	ctr_internal_create_func( CtrStdBool, ctr_build_string_from_cstring( CTR_DICT_SYMBOL_EQUALS ),&ctr_bool_eq );
	ctr_internal_create_func( CtrStdBool, ctr_build_string_from_cstring( CTR_DICT_UNEQUALS_SYMBOL ), &ctr_bool_neq );
	ctr_internal_create_func( CtrStdBool, ctr_build_string_from_cstring( CTR_DICT_TONUMBER ), &ctr_bool_to_number );
	ctr_internal_create_func( CtrStdBool, ctr_build_string_from_cstring( CTR_DICT_TOSTRING ), &ctr_bool_to_string );
	ctr_internal_create_func( CtrStdBool, ctr_build_string_from_cstring( CTR_DICT_EITHEROR ), &ctr_bool_either_or );
	ctr_internal_create_func( CtrStdBool, ctr_build_string_from_cstring( CTR_DICT_COPY ), &ctr_bool_copy );
	ctr_internal_create_func( CtrStdBool, ctr_build_string_from_cstring( CTR_DICT_CODE ), &ctr_bool_to_string );

	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_BOOLEAN ), CtrStdBool, 0 );
	CtrStdBool->link = CtrStdObject;
	CtrStdBool->info.sticky = 1;
	
	CtrStdBoolTrue = ctr_build_bool(1);
	CtrStdBoolTrue->info.sticky = 1;
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_TRUE ), CtrStdBoolTrue, 0 );
	CtrStdBoolFalse = ctr_build_bool(0);
	CtrStdBoolFalse->info.sticky = 1;
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_FALSE ), CtrStdBoolFalse, 0 );
	

	/* Number */
	CtrStdNumber = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNUMBER);
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_number_new );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_PLUS ), &ctr_number_add );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_ADD_SET ), &ctr_number_inc );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_MINUS ), &ctr_number_minus );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_SUBTRACT_SET ), &ctr_number_dec );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_MULTIPLIER ),&ctr_number_multiply );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_MULTIPLIER_SET ),&ctr_number_mul );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_DIVISION ), &ctr_number_divide );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_DIVIDER_SET ), &ctr_number_div );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_GREATER ), &ctr_number_higherThan );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_GREATER_OR_EQUAL_SYMBOL ), &ctr_number_higherEqThan );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_LESS ), &ctr_number_lowerThan );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_LESS_OR_EQUAL_SYMBOL ), &ctr_number_lowerEqThan );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_SYMBOL_EQUALS ), &ctr_number_eq );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_UNEQUALS_SYMBOL ), &ctr_number_neq );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_MODULO ), &ctr_number_modulo );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_FLOOR ), &ctr_number_floor );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_CEIL ), &ctr_number_ceil );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_ROUND ), &ctr_number_round );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_ABS ), &ctr_number_abs );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_SQRT ), &ctr_number_sqrt );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_POWER ), &ctr_number_pow );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_ODD ), &ctr_number_odd );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_EVEN ), &ctr_number_even );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_POS ), &ctr_number_positive );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_NEG ), &ctr_number_negative );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_TOSTRING ), &ctr_number_to_string );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_RAW ), &ctr_number_to_string_flat );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_TOBOOL ), &ctr_number_to_boolean );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_TONUMBER ), &ctr_object_myself );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_BETWEEN ),&ctr_number_between );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_QUALIFY ),&ctr_number_qualify );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_QUALIFICATION ),&ctr_number_qualification );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_RESPOND_TO ),&ctr_number_qualify );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_COPY ), &ctr_number_copy );
	ctr_internal_create_func(CtrStdNumber, ctr_build_string_from_cstring( CTR_DICT_CODE ), &ctr_number_to_string );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_NUMBER ), CtrStdNumber, 0);
	CtrStdNumber->link = CtrStdObject;
	CtrStdNumber->info.sticky = 1;

	/* String */
	CtrStdString = ctr_internal_create_object(CTR_OBJECT_TYPE_OTSTRING);
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_string_new );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_LENGTH ), &ctr_string_length );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_FROM_LENGTH ), &ctr_string_from_length );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_PLUS ), &ctr_string_concat );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_SYMBOL_EQUALS ), &ctr_string_eq );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_UNEQUALS_SYMBOL ), &ctr_string_neq );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_TRIM ), &ctr_string_trim );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_CHAR_AT_SET ), &ctr_string_at );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_INDEX_OF ), &ctr_string_index_of );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_LAST_INDEX_OF ), &ctr_string_last_index_of );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_REPLACE_WITH ), &ctr_string_replace_with );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_SPLIT ), &ctr_string_split );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_ASCII_UPPER_CASE ), &ctr_string_to_upper );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_ASCII_LOWER_CASE ), &ctr_string_to_lower );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_SKIP ), &ctr_string_skip );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_APPEND ), &ctr_string_append );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_ADD_SET ), &ctr_string_append );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_TONUMBER ), &ctr_string_to_number );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_ITONUMBER ), &ctr_string_in_to_number );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_TOBOOL ), &ctr_string_to_boolean );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_HASH_WITH_KEY ), &ctr_string_hash_with_key );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_TOSTRING), &ctr_string_to_string );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_CHARACTERS ),&ctr_string_characters );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_CONTAINS ),&ctr_string_contains );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_COMPARE_SET ),&ctr_string_compare );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_LESS ),&ctr_string_before );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_LESS_OR_EQUAL_SYMBOL ),&ctr_string_before_or_same );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_GREATER ),&ctr_string_after );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_GREATER_OR_EQUAL_SYMBOL ),&ctr_string_after_or_same);
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_RESPOND_TO_AND ),&ctr_string_fill_in);
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_COPY ), &ctr_string_copy );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_CODE ), &ctr_string_to_code );
	ctr_internal_create_func(CtrStdString, ctr_build_string_from_cstring( CTR_DICT_TOOBJECT ), &ctr_string_eval );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_STRING ), CtrStdString, 0 );
	CtrStdString->link = CtrStdObject;
	CtrStdString->info.sticky = 1;

	/* Block */
	CtrStdBlock = ctr_internal_create_object(CTR_OBJECT_TYPE_OTBLOCK);
	ctr_internal_create_func(CtrStdBlock, ctr_build_string_from_cstring( CTR_DICT_RUN ), &ctr_block_runIt );
	ctr_internal_create_func(CtrStdBlock, ctr_build_string_from_cstring( CTR_DICT_APPLY_TO ), &ctr_block_runIt );
	ctr_internal_create_func(CtrStdBlock, ctr_build_string_from_cstring( CTR_DICT_APPLY_TO_AND ), &ctr_block_runIt );
	ctr_internal_create_func(CtrStdBlock, ctr_build_string_from_cstring( CTR_DICT_APPLY_TO_AND_AND ), &ctr_block_runIt );
	ctr_internal_create_func(CtrStdBlock, ctr_build_string_from_cstring( CTR_DICT_SET_VALUE ), &ctr_block_set );
	ctr_internal_create_func(CtrStdBlock, ctr_build_string_from_cstring( CTR_DICT_ERROR ), &ctr_block_error );
	ctr_internal_create_func(CtrStdBlock, ctr_build_string_from_cstring( CTR_DICT_CATCH ), &ctr_block_catch );
	ctr_internal_create_func(CtrStdBlock, ctr_build_string_from_cstring( CTR_DICT_MULTIPLIER ), &ctr_block_times );
	ctr_internal_create_func(CtrStdBlock, ctr_build_string_from_cstring( CTR_DICT_PROCEDURE ), &ctr_block_procedure );
	ctr_internal_create_func(CtrStdBlock, ctr_build_string_from_cstring( CTR_DICT_WHILE ), &ctr_block_while_true );
	ctr_internal_create_func(CtrStdBlock, ctr_build_string_from_cstring( CTR_DICT_TOSTRING ), &ctr_block_to_string );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_CODE_BLOCK ), CtrStdBlock, 0 );
	CtrStdBlock->link = CtrStdObject;
	CtrStdBlock->info.sticky = 1;

	/* Array */
	CtrStdArray = ctr_array_new(CtrStdObject, NULL);
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_array_new );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_NEW_ARRAY_AND_PUSH_SYMBOL ), &ctr_array_new_and_push );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_TYPE ), &ctr_array_type );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_APPEND ), &ctr_array_push );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_BULLET ), &ctr_array_push );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_PUSH_SYMBOL ), &ctr_array_push );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_PREPEND ), &ctr_array_unshift );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_SHIFT ), &ctr_array_shift );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_COUNT ), &ctr_array_count );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_JOIN ), &ctr_array_join );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_POP ), &ctr_array_pop );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_POSITION_SET ), &ctr_array_get );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_AT_SYMBOL ), &ctr_array_get );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_SORT ), &ctr_array_sort );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_PUT_AT ), &ctr_array_put );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_FROM_LENGTH ), &ctr_array_from_length );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_SPLICE ), &ctr_array_splice );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_PLUS ), &ctr_array_add );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_MAP ), &ctr_array_map );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_EACH ), &ctr_array_map );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_MIN ), &ctr_array_min );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_MAX ), &ctr_array_max );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_TOSTRING ), &ctr_array_to_string );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_LAST ), &ctr_array_last );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_SECOND_LAST ), &ctr_array_second_last );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_FIRST ), &ctr_array_first );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_FILL_WITH ), &ctr_array_fill );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_MINUS ), &ctr_array_delete );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_BY_SET ), &ctr_array_combine );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_INDEX_OF ), &ctr_array_index_of );
	ctr_internal_create_func(CtrStdArray, ctr_build_string_from_cstring( CTR_DICT_COPY ), &ctr_array_copy );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_LIST_OBJECT ), CtrStdArray, 0 );
	CtrStdArray->link = CtrStdObject;
	CtrStdArray->info.sticky = 1;

	/* Map */
	CtrStdMap = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_map_new );
	ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring( CTR_DICT_TYPE ), &ctr_map_type );
	ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring( CTR_DICT_PUT_AT ), &ctr_map_put );
	ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring( CTR_DICT_AT ), &ctr_map_get );
	ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring( CTR_DICT_AT_SYMBOL ), &ctr_map_get );
	ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring( CTR_DICT_COUNT ), &ctr_map_count );
	ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring( CTR_DICT_EACH ), &ctr_map_each );
	ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring( CTR_DICT_MAP ), &ctr_map_each );
	ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring( CTR_DICT_TOSTRING ), &ctr_map_to_string );
	ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring( CTR_DICT_MINUS ), &ctr_map_delete );
	ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring( CTR_DICT_ENTRIES ), &ctr_map_keys );
	ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring( CTR_DICT_VALUES ), &ctr_map_values );
	ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring( CTR_DICT_HAS ), &ctr_map_has );
	ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring( CTR_DICT_RESPOND_TO_AND ), &ctr_map_key_value );
	ctr_internal_create_func(CtrStdMap, ctr_build_string_from_cstring( CTR_DICT_RESPOND_TO ), &ctr_map_get );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MAP_OBJECT ), CtrStdMap, 0 );
	CtrStdMap->link = CtrStdObject;
	CtrStdMap->info.sticky = 1;

	/* Console */
	CtrStdConsole = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	ctr_internal_create_func(CtrStdConsole, ctr_build_string_from_cstring( CTR_DICT_WRITE ), &ctr_console_write );
	ctr_internal_create_func(CtrStdConsole, ctr_build_string_from_cstring( CTR_DICT_STOP ), &ctr_console_brk );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_PEN_ICON ), CtrStdConsole, 0 );
	CtrStdConsole->link = CtrStdObject;
	CtrStdConsole->info.sticky = 1;

	/* File */
	CtrStdFile = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	CtrStdFile->value.rvalue = NULL;
	if (!ctr_flag_sandbox) {
	ctr_internal_create_func(CtrStdFile, ctr_build_string_from_cstring( CTR_DICT_NEW_SET ), &ctr_file_new );
	ctr_internal_create_func(CtrStdFile, ctr_build_string_from_cstring( CTR_DICT_PATH ), &ctr_file_path );
	ctr_internal_create_func(CtrStdFile, ctr_build_string_from_cstring( CTR_DICT_READ ), &ctr_file_read );
	ctr_internal_create_func(CtrStdFile, ctr_build_string_from_cstring( CTR_DICT_WRITE ), &ctr_file_write );
	ctr_internal_create_func(CtrStdFile, ctr_build_string_from_cstring( CTR_DICT_APPEND ), &ctr_file_append );
	ctr_internal_create_func(CtrStdFile, ctr_build_string_from_cstring( CTR_DICT_EXISTS ), &ctr_file_exists );
	ctr_internal_create_func(CtrStdFile, ctr_build_string_from_cstring( CTR_DICT_SIZE ), &ctr_file_size );
	ctr_internal_create_func(CtrStdFile, ctr_build_string_from_cstring( CTR_DICT_DELETE ), &ctr_file_delete );
	ctr_internal_create_func(CtrStdFile, ctr_build_string_from_cstring( CTR_DICT_OPEN ), &ctr_file_open );
	ctr_internal_create_func(CtrStdFile, ctr_build_string_from_cstring( CTR_DICT_CLOSE ), &ctr_file_close );
	ctr_internal_create_func(CtrStdFile, ctr_build_string_from_cstring( CTR_DICT_LIST ), &ctr_file_list );
	ctr_internal_create_func(CtrStdFile, ctr_build_string_from_cstring( CTR_DICT_TOSTRING ), &ctr_file_to_string );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_FILE ), CtrStdFile, 0);
	}
	CtrStdFile->link = CtrStdObject;
	CtrStdFile->info.sticky = 1;

	/* Command */
	CtrStdCommand = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	ctr_internal_create_func(CtrStdCommand, ctr_build_string_from_cstring( CTR_DICT_ARGUMENT ), &ctr_program_argument );
	ctr_internal_create_func(CtrStdCommand, ctr_build_string_from_cstring( CTR_DICT_ARGUMENT_COUNT ), &ctr_program_num_of_args );
	ctr_internal_create_func(CtrStdCommand, ctr_build_string_from_cstring( CTR_DICT_ENVIRONMENT_VARIABLE ), &ctr_program_get_env );
	if (!ctr_flag_sandbox) {
	ctr_internal_create_func(CtrStdCommand, ctr_build_string_from_cstring( CTR_DICT_WAIT_FOR_INPUT ), &ctr_program_waitforinput );
	ctr_internal_create_func(CtrStdCommand, ctr_build_string_from_cstring( CTR_DICT_SET_ENVIRONMENT_VARIABLE ), &ctr_program_set_env );
	ctr_internal_create_func(CtrStdCommand, ctr_build_string_from_cstring( CTR_DICT_SWEEP ), &ctr_gc_collect );
	ctr_internal_create_func(CtrStdCommand, ctr_build_string_from_cstring( CTR_DICT_MEMORY_LIMIT ), &ctr_gc_setmemlimit );
	ctr_internal_create_func(CtrStdCommand, ctr_build_string_from_cstring( CTR_DICT_MODE ),  &ctr_gc_setmode );
	ctr_internal_create_func(CtrStdCommand, ctr_build_string_from_cstring( CTR_DICT_SHELL ), &ctr_program_shell );
	}
	ctr_internal_create_func(CtrStdCommand, ctr_build_string_from_cstring( CTR_DICT_INPUT ), &ctr_program_input );
	ctr_internal_create_func(CtrStdCommand, ctr_build_string_from_cstring( CTR_DICT_END ), &ctr_program_exit );
	ctr_internal_create_func(CtrStdCommand, ctr_build_string_from_cstring( CTR_DICT_FLUSH ), &ctr_program_flush );
	ctr_internal_create_func(CtrStdCommand, ctr_build_string_from_cstring( CTR_DICT_ERROR ), &ctr_program_err );
	ctr_internal_create_func(CtrStdCommand, ctr_build_string_from_cstring( CTR_DICT_MEMORY ), &ctr_gc_memory );
	ctr_internal_create_func(CtrStdCommand, ctr_build_string_from_cstring( CTR_DICT_USE_SET ), &ctr_program_include );
	ctr_internal_create_func(CtrStdCommand, ctr_build_string_from_cstring( CTR_DICT_TONUMBER ), &ctr_program_tonumber );
	ctr_internal_create_func(CtrStdCommand, ctr_build_string_from_cstring( CTR_DICT_TOSTRING ), &ctr_program_tostring );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_COMP_ICON ), CtrStdCommand, 0 );
	CtrStdCommand->link = CtrStdObject;
	CtrStdCommand->info.sticky = 1;

	/* Clock */
	CtrStdClock = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	if (!ctr_flag_sandbox) {
		ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_WAIT ), &ctr_clock_wait );
	}
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_TIME ), &ctr_clock_time );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_NEW_SET ), &ctr_clock_new_set );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_clock_new );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_DAY ), &ctr_clock_day );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_SET_DAY ), &ctr_clock_set_day );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_MONTH ), &ctr_clock_month );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_SET_MONTH ), &ctr_clock_set_month );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_YEAR ), &ctr_clock_year );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_SET_YEAR ), &ctr_clock_set_year );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_HOUR ), &ctr_clock_hour );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_SET_HOUR ), &ctr_clock_set_hour );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_MINUTE ), &ctr_clock_minute );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_SET_MINUTE ), &ctr_clock_set_minute );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_SECOND ), &ctr_clock_second );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_SET_SECOND ), &ctr_clock_set_second );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_WEEK_DAY ), &ctr_clock_weekday );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_YEAR_DAY ), &ctr_clock_yearday );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_WEEK ), &ctr_clock_week );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_ZONE ), &ctr_clock_get_zone );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_ZONE_SET ), &ctr_clock_set_zone );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_FORMAT ), &ctr_clock_format );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_LIKE ), &ctr_clock_like );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_TOSTRING ), &ctr_clock_to_string );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_TONUMBER ), &ctr_clock_to_number );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_ADD_SET ), &ctr_clock_add );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_SUBTRACT_SET ), &ctr_clock_subtract );
	ctr_internal_create_func(CtrStdClock, ctr_build_string_from_cstring( CTR_DICT_COPY ), &ctr_clock_copy );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_CLOCK_ICON ), CtrStdClock, 0 );
	ctr_clock_init( CtrStdClock );
	CtrStdClock->link = CtrStdObject;
	CtrStdFile->info.sticky = 1;
	
	/* Other objects */
	CtrStdBreak = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	CtrStdContinue = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	CtrStdExit = ctr_internal_create_object( CTR_OBJECT_TYPE_OTOBJECT );
	CtrStdBreak->info.sticky = 1;
	CtrStdContinue->info.sticky = 1;
	CtrStdExit->info.sticky = 1;

	/* Create a dummy variable to balance memory use in tests. */
	ctr_internal_object_set_property( ctr_contexts[ctr_context_id], ctr_build_string_from_cstring(".rs"), ctr_build_number_from_float(1), CTR_CATEGORY_PRIVATE_PROPERTY );
}

/**
 * ?internal
 *
 * CTRMessageSend
 *
 * Sends a message to a receiver object.
 */
ctr_object* ctr_send_message(ctr_object* receiverObject, char* message, long vlen, ctr_argument* argumentList) {
	char toParent = 0;
	ctr_object* me = NULL;
	ctr_object* methodObject;
	ctr_object* searchObject;
	ctr_object* returnValue;
	ctr_argument* argCounter;
	ctr_argument* mesgArgument;
	ctr_object* result;
	ctr_object* (*funct)(ctr_object* receiverObject, ctr_argument* argumentList);
	ctr_object* msg = NULL;
	int argCount;
	if (CtrStdFlow != NULL) return CtrStdNil; /* Error mode, ignore subsequent messages until resolved. */
	methodObject = NULL;
	searchObject = receiverObject;
	if (vlen > 1 && message[0] == '`') {
		int j = ctr_context_id;
		while( !me && j ) {
			me = ctr_internal_object_find_property(ctr_contexts[j--], ctr_build_string_from_cstring( ctr_clex_keyword_my_icon ), 0);
		}
		if (me) {
			searchObject = me;
			toParent = 1;
			message = message + 1;
			vlen--;
		}
	}
	msg = ctr_build_string(message, vlen);
	msg->info.sticky = 1; /* prevent message from being swept, no need to free(), GC will do */
	while(!methodObject) {
		methodObject = ctr_internal_object_find_property(searchObject, msg, 1);
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
		mesgArgument = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		mesgArgument->object = ctr_build_string(message, vlen);
		mesgArgument->next = argumentList;
		if (argCount == 0 || argCount > 3) {
			returnValue = ctr_send_message(receiverObject, CTR_DICT_RESPOND_TO, strlen(CTR_DICT_RESPOND_TO),  mesgArgument);
		} else if (argCount == 1) {
			returnValue = ctr_send_message(receiverObject, CTR_DICT_RESPOND_TO_AND, strlen(CTR_DICT_RESPOND_TO_AND),  mesgArgument);
		} else if (argCount == 2) {
			returnValue = ctr_send_message(receiverObject, CTR_DICT_RESPOND_TO_AND_AND, strlen(CTR_DICT_RESPOND_TO_AND_AND),  mesgArgument);
		} else if (argCount == 3) {
			returnValue = ctr_send_message(receiverObject, CTR_DICT_RESPOND_TO_AND_AND_AND, strlen(CTR_DICT_RESPOND_TO_AND_AND_AND),  mesgArgument);
		}
		ctr_heap_free( mesgArgument );
		msg->info.sticky = 0;
		if (receiverObject->info.chainMode == 1) return receiverObject;
		return returnValue;
	}
	if (methodObject->info.type == CTR_OBJECT_TYPE_OTNATFUNC) {
		funct = methodObject->value.fvalue;
		if (ctr_deserialize_mode
				&& funct != ctr_array_push
				&& funct != ctr_array_new_and_push
				&& funct != ctr_map_new
				&& funct != ctr_map_put
				&& funct != ctr_array_new
				&& funct != ctr_nil_to_string
				&& funct != ctr_bool_to_string
				&& funct != ctr_number_to_string
				&& funct != ctr_string_to_string
			) {
			result = CtrStdNil;
		} else {
			result = funct(receiverObject, argumentList);
		}
	}
	if (methodObject->info.type == CTR_OBJECT_TYPE_OTBLOCK && methodObject->info.selfbind == 1) {
		result = ctr_block_run(methodObject, argumentList, receiverObject);
	}
	if (methodObject->info.type == CTR_OBJECT_TYPE_OTBLOCK && methodObject->info.selfbind == 0) {
		result = ctr_block_run(methodObject, argumentList, NULL);
	}
	if (msg) msg->info.sticky = 0;
	if (receiverObject->info.chainMode == 1) return receiverObject;
	return result;
}


/**
 * ?internal
 *
 * CTRValueAssignment
 *
 * Assigns a value to a variable in the current context.
 */
ctr_object* ctr_assign_value(ctr_object* key, ctr_object* o) {
	ctr_object* object = NULL;
	if (CtrStdFlow) return CtrStdNil;
	key->info.sticky = 0;
	object = o;
	ctr_set(key, object);
	return object;
}


/**
 * ?internal
 *
 * CTRAssignValueObject
 *
 * Assigns a value to a property of an object.
 */
ctr_object* ctr_assign_value_to_my(ctr_object* key, ctr_object* o) {
	ctr_object* object = NULL;
	ctr_object* my = ctr_find(ctr_build_string_from_cstring( ctr_clex_keyword_me_icon ) );
	if (CtrStdFlow) return CtrStdNil;
	key->info.sticky = 0;
	object = o;
	ctr_internal_object_set_property(my, key, object, 0);
	return object;
}

/**
 * ?internal
 *
 * CTRAssignValueObjectLocal
 *
 * Assigns a value to a local of an object.
 */
ctr_object* ctr_assign_value_to_local(ctr_object* key, ctr_object* o) {
	ctr_object* object = NULL;
	ctr_object* context;
	if (CtrStdFlow) return CtrStdNil;
	context = ctr_contexts[ctr_context_id];
	key->info.sticky = 0;
	object = o;
	ctr_internal_object_set_property(context, key, object, 0);
	return object;
}

