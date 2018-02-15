#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jsmn/jsmn.h"
#include "../../citrine.h"

ctr_object* ctr_json_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* jsonInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	jsonInstance->link = myself;
	return jsonInstance;
}

ctr_object* ctr_jsmn_dump( char* data, jsmntok_t* t ) {
	ctr_object* answer;
	ctr_argument* a;
	ctr_argument* b;
	char c;
	a = NULL;
	b = NULL;
	int i;
	if (t->type == JSMN_STRING) {
		answer = ctr_build_string( (data + t->start), (t->end - t->start) );
	}
	if (t->type == JSMN_PRIMITIVE ) {
		c = *(data + t->start);
		if ( c == 't' ) {
			answer = ctr_build_bool( 1 );
		} else if ( c == 'f' ) {
			answer = ctr_build_bool( 0 );
		} else if ( c == 'n' ) {
			answer = ctr_build_nil();
		} else if ( c == '-' || ( c >= '0' && c <= '9' ) ) {
			answer = ctr_string_to_number( ctr_build_string( (char*) (data + t->start), (t->end - t->start) ), a );
		}
	}
	if (t->type == JSMN_UNDEFINED) {
		answer = CtrStdNil;
	}
	if (t->type == JSMN_ARRAY) {
		a = ctr_heap_allocate( sizeof(ctr_argument) );
		answer = ctr_array_new( CtrStdArray, NULL );
		for(i = 1; i<=t->size; i++ ) {
			ctr_object* element = ctr_jsmn_dump( data, t+i );
			a->object = element;
			ctr_array_push( answer, a );
		}
		ctr_heap_free(a);
	}
	if (t->type == JSMN_OBJECT ) {
		a = ctr_heap_allocate( sizeof(ctr_argument) );
		b = ctr_heap_allocate( sizeof(ctr_argument) );
		answer = ctr_map_new( CtrStdMap, a );
		for (i = 1; i<(t->size*2); i+=2) {
			ctr_object* property = ctr_jsmn_dump( data, t+i );
			ctr_object* value = ctr_jsmn_dump( data, t+i+1 );
			a->object = value;
			b->object = property;
			a->next = b;
			ctr_map_put( answer, a );
		}
		ctr_heap_free(a);
		ctr_heap_free(b);
	}
	return answer;
}


ctr_object* ctr_json_parse(ctr_object* myself, ctr_argument* argumentList) {
	char* jsonString = ctr_heap_allocate_cstring(
		ctr_internal_cast2string( argumentList->object )
	);
	ctr_size size;
	jsmn_parser jsmn;
	jsmn_parser jsmn2;
	ctr_object* answer;
	jsmntok_t* t;
	int r;
	jsmn_init(&jsmn);
	size = (ctr_size) jsmn_parse(&jsmn, jsonString, strlen(jsonString), NULL, 0);
	t = (jsmntok_t*) ctr_heap_allocate( sizeof(jsmntok_t) * size );
	jsmn_init(&jsmn2);
	r = jsmn_parse(&jsmn2, jsonString, strlen(jsonString), t, size);
	if (r < size || t[0].type != JSMN_OBJECT) {
		ctr_heap_free( t );
		ctr_heap_free( jsonString );
		return CtrStdNil;
	}
	answer = ctr_jsmn_dump(jsonString, t);
	ctr_heap_free( t );
	ctr_heap_free( jsonString );
	return answer;
}


void begin(){
	ctr_object* jsonObject = ctr_json_new(CtrStdObject, NULL);
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "parse:" ), &ctr_json_parse );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "Jsmn" ), jsonObject, CTR_CATEGORY_PUBLIC_PROPERTY);
}
