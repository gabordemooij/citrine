
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>

#include "citrine.h"
#include "lib/utf8proc-1.3.1/utf8proc.h"

obj* World = NULL;
obj* contexts[100];
int cid = 0;
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
obj* CCommand;
obj* CShell;
obj* CCoin;
obj* CClock;

int gc_dust = 0;
int gc_object_count = 0;
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


char* ctr_internal_memmem(char* haystack, long hlen, char* needle, long nlen, int reverse ) {
	char* cur;
	char* last;
	char* begin;
	int step = (1 - reverse * 2); //1 if reverse = 0, -1 if reverse = 1
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

obj* ctr_internal_create_object(int type) {
	obj* o = malloc(sizeof(obj));
	o->properties = malloc(sizeof(ctr_map));
	o->methods = malloc(sizeof(ctr_map));
	o->properties->size = 0;
	o->methods->size = 0;
	o->properties->head = NULL;
	o->methods->head = NULL;
	o->info.type = type;
	o->info.sticky = 1;
	o->info.mark = 0;
	if (type==OTBOOL) o->value.bvalue = 0;
	if (type==OTNUMBER) o->value.nvalue = 0;
	if (type==OTSTRING) {
		o->value.svalue = malloc(sizeof(cstr));
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

void ctr_internal_create_func(obj* o, obj* key, void* f ) {
	obj* methodObject = ctr_internal_create_object(OTNATFUNC);
	methodObject->value.rvalue = (void*) f;
	ctr_internal_object_add_property(o, key, methodObject, 1);
}

obj* ctr_internal_cast2number(obj* o) {
	if (o->info.type == OTNUMBER) return o;
	if (o->info.type == OTSTRING) {
		char* cstring = malloc((o->value.svalue->vlen+1)*sizeof(char));
		memcpy(cstring, o->value.svalue->value, o->value.svalue->vlen);
		memcpy((char*)((long)cstring+(o->value.svalue->vlen*sizeof(char))),"\0", sizeof(char));
		return ctr_build_number(cstring);
	}
	return ctr_build_number("0");
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




#include "base.c"
#include "system.c"
#include "collections.c"
#include "file.c"


void ctr_initialize_world() {
	srand((unsigned)time(NULL));
	
	ctr_first_object = NULL;
	
	World = ctr_internal_create_object(OTOBJECT);
	contexts[0] = World;
	Object = ctr_internal_create_object(OTOBJECT);
	ctr_internal_create_func(Object, ctr_build_string("new", 3), &ctr_object_make);
	ctr_internal_create_func(Object, ctr_build_string("equals:", 7), &ctr_object_equals);
	ctr_internal_create_func(Object, ctr_build_string("on:do:", 6), &ctr_object_on_do);
	ctr_internal_create_func(Object, ctr_build_string("override:do:", 12), &ctr_object_override_does);
	ctr_internal_create_func(Object, ctr_build_string("basedOn:", 8), &ctr_object_basedOn);
	ctr_internal_create_func(Object, ctr_build_string("respondTo:", 10), &ctr_object_respond);
	ctr_internal_create_func(Object, ctr_build_string("respondTo:with:", 15), &ctr_object_respond);
	ctr_internal_create_func(Object, ctr_build_string("respondTo:with:and:", 19), &ctr_object_respond);
	ctr_internal_create_func(Object, ctr_build_string("new", 3), &ctr_object_make);
	Object->link = NULL;
	ctr_internal_object_add_property(World, ctr_build_string("Object", 6), Object, 0);
	Console = ctr_internal_create_object(OTOBJECT);
	ctr_internal_create_func(Console, ctr_build_string("write:", 6), &ctr_console_write);
	ctr_internal_create_func(Console, ctr_build_string("brk", 3), &ctr_console_brk);
	ctr_internal_object_add_property(World, ctr_build_string("Pen", 3), Console, 0);
	Console->link = Object;
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
	ctr_internal_create_func(Number, ctr_build_string("≥",3),&ctr_number_higherEqThan);
	ctr_internal_create_func(Number, ctr_build_string("<",1),&ctr_number_lowerThan);
	ctr_internal_create_func(Number, ctr_build_string("≤",3),&ctr_number_lowerEqThan);
	ctr_internal_create_func(Number, ctr_build_string("=",1),&ctr_number_eq);
	ctr_internal_create_func(Number, ctr_build_string("≠",3),&ctr_number_neq);
	ctr_internal_create_func(Number, ctr_build_string("%",1),&ctr_number_modulo);
	ctr_internal_create_func(Number, ctr_build_string("times:",6),&ctr_number_times);
	ctr_internal_create_func(Number, ctr_build_string("factorial",9),&ctr_number_factorial);
	ctr_internal_create_func(Number, ctr_build_string("floor",5),&ctr_number_floor);
	ctr_internal_create_func(Number, ctr_build_string("ceil",4),&ctr_number_ceil);
	ctr_internal_create_func(Number, ctr_build_string("round",5),&ctr_number_round);
	ctr_internal_create_func(Number, ctr_build_string("abs",3),&ctr_number_abs);
	ctr_internal_create_func(Number, ctr_build_string("sin",3),&ctr_number_sin);
	ctr_internal_create_func(Number, ctr_build_string("cos",3),&ctr_number_cos);
	ctr_internal_create_func(Number, ctr_build_string("exp",3),&ctr_number_exp);
	ctr_internal_create_func(Number, ctr_build_string("sqrt",4),&ctr_number_sqrt);
	ctr_internal_create_func(Number, ctr_build_string("tan",3),&ctr_number_tan);
	ctr_internal_create_func(Number, ctr_build_string("atan",4),&ctr_number_atan);
	ctr_internal_create_func(Number, ctr_build_string("log",3),&ctr_number_log);
	ctr_internal_create_func(Number, ctr_build_string("pow:",4),&ctr_number_pow);
	ctr_internal_create_func(Number, ctr_build_string("min:",4),&ctr_number_min);
	ctr_internal_create_func(Number, ctr_build_string("max:",4),&ctr_number_max);
	ctr_internal_create_func(Number, ctr_build_string("between:and:",12),&ctr_number_between);
	ctr_internal_object_add_property(World, ctr_build_string("Number", 6), Number, 0);
	Number->link = Object;
	CDice = ctr_internal_create_object(OTOBJECT);
	ctr_internal_create_func(CDice, ctr_build_string("roll", 4), &ctr_dice_throw);
	ctr_internal_create_func(CDice, ctr_build_string("rollWithSides:", 14), &ctr_dice_sides);
	ctr_internal_object_add_property(World, ctr_build_string("Dice", 4), CDice, 0);
	CDice->link = Object;
	CCoin = ctr_internal_create_object(OTOBJECT);
	ctr_internal_create_func(CCoin, ctr_build_string("flip", 4), &ctr_coin_flip);
	ctr_internal_object_add_property(World, ctr_build_string("Coin", 4), CCoin, 0);
	CCoin->link = Object;
	CClock = ctr_internal_create_object(OTOBJECT);
	ctr_internal_create_func(CClock, ctr_build_string("wait:", 5), &ctr_clock_wait);
	ctr_internal_object_add_property(World, ctr_build_string("Clock", 5), CClock, 0);
	CClock->link = Object;
	CCommand = ctr_internal_create_object(OTOBJECT);
	ctr_internal_create_func(CCommand, ctr_build_string("argument:", 9), &ctr_command_argument);
	ctr_internal_create_func(CCommand, ctr_build_string("argCount", 8), &ctr_command_num_of_args);
	ctr_internal_object_add_property(World, ctr_build_string("Command", 7), CCommand, 0);
	CCommand->link = Object;
	CShell = ctr_internal_create_object(OTOBJECT);
	ctr_internal_create_func(CShell, ctr_build_string("call:", 5), &ctr_shell_call);
	ctr_internal_object_add_property(World, ctr_build_string("Shell", 5), CShell, 0);
	TextString = ctr_internal_create_object(OTSTRING);
	ctr_internal_create_func(TextString, ctr_build_string("bytes", 5), &ctr_string_bytes);
	ctr_internal_create_func(TextString, ctr_build_string("length", 6), &ctr_string_length);
	ctr_internal_create_func(TextString, ctr_build_string("from:to:", 8), &ctr_string_fromto);
	ctr_internal_create_func(TextString, ctr_build_string("from:length:", 12), &ctr_string_from_length);
	ctr_internal_create_func(TextString, ctr_build_string("+", 1), &ctr_string_concat);
	ctr_internal_create_func(TextString, ctr_build_string("=", 1), &ctr_string_eq);
	ctr_internal_create_func(TextString, ctr_build_string("≠", 3), &ctr_string_neq);
	ctr_internal_create_func(TextString, ctr_build_string("trim", 4), &ctr_string_trim);
	ctr_internal_create_func(TextString, ctr_build_string("ltrim", 5), &ctr_string_ltrim);
	ctr_internal_create_func(TextString, ctr_build_string("rtrim", 5), &ctr_string_rtrim);
	ctr_internal_create_func(TextString, ctr_build_string("toUpperCase", 11), &ctr_string_to_upper);
	ctr_internal_create_func(TextString, ctr_build_string("toLowerCase", 11), &ctr_string_to_lower);
	ctr_internal_create_func(TextString, ctr_build_string("nfc", 3), &ctr_string_nfc);
	ctr_internal_create_func(TextString, ctr_build_string("nfk", 3), &ctr_string_nfk);
	ctr_internal_create_func(TextString, ctr_build_string("esc", 3), &ctr_string_html_escape);
	ctr_internal_create_func(TextString, ctr_build_string("at:", 3), &ctr_string_at);
	ctr_internal_create_func(TextString, ctr_build_string("byteAt:", 7), &ctr_string_byte_at);
	ctr_internal_create_func(TextString, ctr_build_string("indexOf:", 8), &ctr_string_index_of);
	ctr_internal_create_func(TextString, ctr_build_string("lastIndexOf:", 12), &ctr_string_last_index_of);
	ctr_internal_create_func(TextString, ctr_build_string("replace:with:", 13), &ctr_string_replace_with);
	ctr_internal_create_func(TextString, ctr_build_string("split:", 6), &ctr_string_split);
	ctr_internal_object_add_property(World, ctr_build_string("String", 6), TextString, 0);
	TextString->link = Object;
	CMap = ctr_internal_create_object(OTOBJECT);
	ctr_internal_create_func(CMap, ctr_build_string("new", 3), &ctr_map_new);
	ctr_internal_create_func(CMap, ctr_build_string("put:at:", 7), &ctr_map_put);
	ctr_internal_create_func(CMap, ctr_build_string("at:", 3), &ctr_map_get);
	ctr_internal_create_func(CMap, ctr_build_string("count", 5), &ctr_map_count);
	ctr_internal_create_func(CMap, ctr_build_string("each:", 5), &ctr_map_each);
	ctr_internal_object_add_property(World, ctr_build_string("Map", 3), CMap, 0);
	CMap->link = Object;
	CArray = ctr_array_new(Object);
	ctr_internal_create_func(CArray, ctr_build_string("new", 3), &ctr_array_new);
	ctr_internal_create_func(CArray, ctr_build_string("←", 3), &ctr_array_new_and_push);
	ctr_internal_create_func(CArray, ctr_build_string("push:", 5), &ctr_array_push);
	ctr_internal_create_func(CArray, ctr_build_string(";", 1), &ctr_array_push);
	ctr_internal_create_func(CArray, ctr_build_string("unshift:", 8), &ctr_array_unshift);
	ctr_internal_create_func(CArray, ctr_build_string("shift", 5), &ctr_array_shift);
	ctr_internal_create_func(CArray, ctr_build_string("count", 5), &ctr_array_count);
	ctr_internal_create_func(CArray, ctr_build_string("join:", 5), &ctr_array_join);
	ctr_internal_create_func(CArray, ctr_build_string("pop", 3), &ctr_array_pop);
	ctr_internal_create_func(CArray, ctr_build_string("at:", 3), &ctr_array_get);
	ctr_internal_create_func(CArray, ctr_build_string("put:at:", 7), &ctr_array_put);
	ctr_internal_object_add_property(World, ctr_build_string("Array", 5), CArray, 0);
	CArray->link = Object;
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
	CBlock = ctr_internal_create_object(OTBLOCK);
	ctr_internal_create_func(CBlock, ctr_build_string("run", 3), &ctr_block_run);
	ctr_internal_create_func(CBlock, ctr_build_string("error:", 6), &ctr_block_error);
	ctr_internal_create_func(CBlock, ctr_build_string("catch:", 6), &ctr_block_catch);
	ctr_internal_object_add_property(World, ctr_build_string("CodeBlock", 9), CBlock, 0);
	CBlock->link = Object;
	BoolX = ctr_internal_create_object(OTBOOL);
	ctr_internal_create_func(BoolX, ctr_build_string("ifTrue:", 7), &ctr_bool_iftrue);
	ctr_internal_create_func(BoolX, ctr_build_string("ifFalse:", 8), &ctr_bool_ifFalse);
	ctr_internal_create_func(BoolX, ctr_build_string("opposite", 8), &ctr_bool_opposite);
	ctr_internal_create_func(BoolX, ctr_build_string("∧", 3), &ctr_bool_and);
	ctr_internal_create_func(BoolX, ctr_build_string("∨", 3), &ctr_bool_or);
	ctr_internal_create_func(BoolX, ctr_build_string("xor:", 4), &ctr_bool_xor);
	ctr_internal_object_add_property(World, ctr_build_string("Boolean", 7), BoolX, 0);
	BoolX->link = Object;
	GC = ctr_internal_create_object(OTOBJECT);
	ctr_internal_create_func(GC, ctr_build_string("sweep", 5), &ctr_gc_collect);
	ctr_internal_create_func(GC, ctr_build_string("dust", 4), &ctr_gc_dust);
	ctr_internal_create_func(GC, ctr_build_string("objectCount", 11), &ctr_gc_object_count);
	ctr_internal_object_add_property(World, ctr_build_string("Broom", 5), GC, 0);
	GC->link = Object;
	Nil = ctr_internal_create_object(OTNIL);
	ctr_internal_object_add_property(World, ctr_build_string("Nil", 3), Nil, 0);
	Nil->link = Object;
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
	key->info.sticky = 0;
	if (o->info.type == OTOBJECT || o->info.type == OTMISC) {
		ctr_set(key, o);
	} else {
		object = ctr_internal_create_object(o->info.type);
		object->properties = o->properties;
		object->methods = o->methods;
		object->link = o->link;
		object->info.sticky = 0;
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
		object->value.avalue = malloc(sizeof(carray));
		object->value.avalue->elements = malloc(o->value.avalue->length*sizeof(obj*));
		object->value.avalue->length = o->value.avalue->length;
		int i;
		obj* putValue;
		for (i = o->value.avalue->tail; i < o->value.avalue->head; i++) {
			putValue = *( (obj**) ((long)o->value.avalue->elements + (i * sizeof(obj*))) );
			*( (obj**) ((long)object->value.avalue->elements + (i * sizeof(obj*))) ) = putValue;
		}
		object->value.avalue->head = o->value.avalue->head;
		object->value.avalue->tail = o->value.avalue->tail;
	 }
	 
	return object;
}

obj* ctr_assign_value_to_my(obj* key, obj* o) {
	obj* object;
	obj* my = ctr_find(ctr_build_string("me", 2));
	key->info.sticky = 0;
	if (o->info.type == OTOBJECT || o->info.type == OTMISC) {
		ctr_internal_object_add_property(my, key, o, 0);
	} else {
		object = ctr_internal_create_object(o->info.type);
		object->properties = o->properties;
		object->methods = o->methods;
		object->link = o->link;
		object->info.sticky = 0;
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
		obj* putValue;
		for (i = 0; i < o->value.avalue->head; i++) {
			putValue = *( (obj**) ((long)o->value.avalue->elements + (i * sizeof(obj*))) );
			*( (obj**) ((long)object->value.avalue->elements + (i * sizeof(obj*))) ) = putValue;
		}
		object->value.avalue->head = o->value.avalue->head;
	 }
	 
	

	return object;
}

