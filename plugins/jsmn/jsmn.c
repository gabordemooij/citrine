#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jsmn/jsmn.h"
#include "../../citrine.h"

/**
 * [Json] new
 *
 * Creates a new instance of the Json object.
 * The Json object allows you to use the Json protocol to communicate with
 * other applications.
 */
ctr_object* ctr_json_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* jsonInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	jsonInstance->link = myself;
	return jsonInstance;
}

/**
 * @internal
 */
ctr_object* ctr_jsmn_dump( char* data, jsmntok_t** tt ) {
	ctr_object* answer;
	ctr_argument* a;
	ctr_argument* b;
	char c;
	a = NULL;
	b = NULL;
	int i;
	jsmntok_t* t = *(tt);
	if (t->type == JSMN_STRING) {
		answer = ctr_build_string( (data + t->start), (t->end - t->start) );
		a = ctr_heap_allocate( sizeof(ctr_argument) );
		a->object = ctr_build_string_from_cstring("\"\t\b\n\r\f\\");
		answer = ctr_string_unescape( answer, a );
		ctr_heap_free(a);
		*(tt)+=1;
	}
	else if (t->type == JSMN_PRIMITIVE ) {
		c = *(data + t->start);
		if ( c == 't' ) {
			answer = ctr_build_bool( 1 );
		} else if ( c == 'f' ) {
			answer = ctr_build_bool( 0 );
		} else if ( c == 'n' ) {
			answer = ctr_build_nil();
		} else if ( c == '-' || ( c >= '0' && c <= '9' ) ) {
			answer = ctr_string_to_number( ctr_build_string( (char*) (data + t->start), (t->end - t->start) ), a );
		} else {
			answer = CtrStdNil;
		}
		*(tt)+=1;
	}
	else if (t->type == JSMN_ARRAY) {
		a = ctr_heap_allocate( sizeof(ctr_argument) );
		answer = ctr_array_new( CtrStdArray, NULL );
		*(tt)+=1;
		for(i = 0; i<t->size; i++ ) {
			ctr_object* element = ctr_jsmn_dump( data, tt );
			a->object = element;
			ctr_array_push( answer, a );
		}
		ctr_heap_free(a);
	}
	else if (t->type == JSMN_OBJECT ) {
		a = ctr_heap_allocate( sizeof(ctr_argument) );
		b = ctr_heap_allocate( sizeof(ctr_argument) );
		answer = ctr_map_new( CtrStdMap, NULL );
		*(tt)+=1;
		for (i = 0; i<(t->size); i+=1) {
			ctr_object* property = ctr_jsmn_dump( data, tt );
			ctr_object* value = ctr_jsmn_dump( data, tt );
			a->object = value;
			b->object = property;
			a->next = b;
			ctr_map_put( answer, a );
		}
		ctr_heap_free(a);
		ctr_heap_free(b);
	}
	else {
		answer = CtrStdNil;
		*(tt)+=1;
	}
	return answer;
}

/**
 * [Json] parse: [String].
 * 
 * Parses a string containing Json encoded data into native Citrine objects.
 * The Json structure must begin with an object (notation: {}). Therefore,
 * the answer to this message from the Json object will always be a Map.
 *
 * Usage:
 *
 * ☞ tasks := '{"todo":7,"finished":3}'.
 * ✎ write: tasks todo. #7
 *
 */
ctr_object* ctr_json_parse(ctr_object* myself, ctr_argument* argumentList) {
	char* jsonString = ctr_heap_allocate_cstring(
		ctr_internal_cast2string( argumentList->object )
	);
	ctr_size size;
	jsmn_parser jsmn;
	jsmn_parser jsmn2;
	ctr_object* answer;
	jsmntok_t* t;
	jsmntok_t* ot;
	int r;
	int s;
	jsmn_init(&jsmn);
	s = jsmn_parse(&jsmn, jsonString, strlen(jsonString), NULL, 0);
	if ( s <= 0 ) {
		ctr_heap_free( jsonString );
		if ( s == JSMN_ERROR_INVAL ) {
			CtrStdFlow = ctr_build_string_from_cstring("Bad token, JSON string is corrupted.");
		}
		return CtrStdNil;
	}
	size = (ctr_size) s;
	t = (jsmntok_t*) ctr_heap_allocate( sizeof(jsmntok_t) * size );
	ot = t;
	jsmn_init(&jsmn2);
	r = jsmn_parse(&jsmn2, jsonString, strlen(jsonString), t, size);
	if (r < size || t[0].type != JSMN_OBJECT) {
		ctr_heap_free( t );
		ctr_heap_free( jsonString );
		return CtrStdNil;
	}
	answer = ctr_jsmn_dump(jsonString, &t);
	ctr_heap_free( ot );
	ctr_heap_free( jsonString );
	return answer;
}

/**
 * [Json] jsonify: [Map].
 *
 * Given a map, the jsonify: message will make the Json object return a
 * String object representing the Map.
 */
ctr_object* ctr_json_jsonify(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object*  string;
	ctr_mapitem* mapItem;
	ctr_argument* newArgumentList;
	string  = ctr_build_string_from_cstring( "{" );
	mapItem = argumentList->object->properties->head;
	newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	while( mapItem ) {
		if ( mapItem->key->info.type == CTR_OBJECT_TYPE_OTBOOL && mapItem->key->value.bvalue == 1) {
			newArgumentList->object = ctr_build_string_from_cstring( "true" );
			ctr_string_append( string, newArgumentList );
		}
		else if ( mapItem->key->info.type == CTR_OBJECT_TYPE_OTBOOL && mapItem->key->value.bvalue == 0) {
			newArgumentList->object = ctr_build_string_from_cstring( "false" );
			ctr_string_append( string, newArgumentList );
		}
		else if ( mapItem->key->info.type == CTR_OBJECT_TYPE_OTNIL ) {
			newArgumentList->object = ctr_build_string_from_cstring( "null" );
			ctr_string_append( string, newArgumentList );
		}
		else if ( mapItem->key->info.type == CTR_OBJECT_TYPE_OTNUMBER ) {
			newArgumentList->object = mapItem->value;
			ctr_string_append( string, newArgumentList );
		}
		else if ( mapItem->key->info.type == CTR_OBJECT_TYPE_OTSTRING ) {
			newArgumentList->object = ctr_build_string_from_cstring( "\"" );
			ctr_string_append( string, newArgumentList );
			newArgumentList->object = ctr_build_string_from_cstring("\"\n\b\r\t\f\\");
			newArgumentList->object = ctr_string_escape( mapItem->key, newArgumentList );
			ctr_string_append( string, newArgumentList );
			newArgumentList->object = ctr_build_string_from_cstring( "\"" );
			ctr_string_append( string, newArgumentList );
		}
		newArgumentList->object = ctr_build_string_from_cstring( ":" );
		ctr_string_append( string, newArgumentList );
		if ( mapItem->value->info.type == CTR_OBJECT_TYPE_OTBOOL && mapItem->value->value.bvalue == 1) {
			newArgumentList->object = ctr_build_string_from_cstring( "true" );
			ctr_string_append( string, newArgumentList );
		}
		else if ( mapItem->value->info.type == CTR_OBJECT_TYPE_OTBOOL && mapItem->value->value.bvalue == 0) {
			newArgumentList->object = ctr_build_string_from_cstring( "false" );
			ctr_string_append( string, newArgumentList );
		}
		else if ( mapItem->value->info.type == CTR_OBJECT_TYPE_OTNIL ) {
			newArgumentList->object = ctr_build_string_from_cstring( "null" );
			ctr_string_append( string, newArgumentList );
		}
		else if ( mapItem->value->info.type == CTR_OBJECT_TYPE_OTNUMBER ) {
			newArgumentList->object = mapItem->value;
			ctr_string_append( string, newArgumentList );
		}
		else if ( mapItem->value->info.type == CTR_OBJECT_TYPE_OTSTRING ) {
			newArgumentList->object = ctr_build_string_from_cstring( "\"" );
			ctr_string_append( string, newArgumentList );
			newArgumentList->object = ctr_build_string_from_cstring("\"\n\b\r\t\f\\");
			newArgumentList->object = ctr_string_escape( mapItem->value, newArgumentList );
			ctr_string_append( string, newArgumentList );
			newArgumentList->object = ctr_build_string_from_cstring( "\"" );
			ctr_string_append( string, newArgumentList );
		}
		else if ( mapItem->value->info.type == CTR_OBJECT_TYPE_OTARRAY ) {
			int i;
			ctr_object* array = mapItem->value;
			ctr_object* arrayElement;
			newArgumentList->object = ctr_build_string_from_cstring( "[" );
			ctr_string_append( string, newArgumentList );
			for(i=array->value.avalue->tail; i<array->value.avalue->head; i++) {
				if ( i > array->value.avalue->tail ) {
					newArgumentList->object = ctr_build_string_from_cstring( "," );
					ctr_string_append( string, newArgumentList );
				}
				arrayElement = *( array->value.avalue->elements + i );
				if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTBOOL && arrayElement->value.bvalue == 1) {
					newArgumentList->object = ctr_build_string_from_cstring( "true" );
					ctr_string_append( string, newArgumentList );
				}
				else if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTBOOL && arrayElement->value.bvalue == 0) {
					newArgumentList->object = ctr_build_string_from_cstring( "false" );
					ctr_string_append( string, newArgumentList );
				}
				else if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTNIL ) {
					newArgumentList->object = ctr_build_string_from_cstring( "null" );
					ctr_string_append( string, newArgumentList );
				}
				else if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTNUMBER ) {
					newArgumentList->object = arrayElement;
					ctr_string_append( string, newArgumentList );
				}
				else if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTSTRING ) {
					newArgumentList->object = ctr_build_string_from_cstring( "\"" );
					ctr_string_append( string, newArgumentList );
					newArgumentList->object = ctr_string_escape( arrayElement, newArgumentList );
					ctr_string_append( string, newArgumentList );
					newArgumentList->object = ctr_build_string_from_cstring( "\"" );
					ctr_string_append( string, newArgumentList );
				}
				else {
					newArgumentList->object = arrayElement;
					newArgumentList->object = ctr_json_jsonify( myself, newArgumentList );
					ctr_string_append( string, newArgumentList );
				}
			}
			newArgumentList->object = ctr_build_string_from_cstring( "]" );
			ctr_string_append( string, newArgumentList );
		}
		else {
			newArgumentList->object = mapItem->value;
			newArgumentList->object = ctr_json_jsonify( myself, newArgumentList );
			ctr_string_append( string, newArgumentList );
		}
		mapItem = mapItem->next;
		if ( mapItem ) {
			newArgumentList->object = ctr_build_string_from_cstring( ", " );
			ctr_string_append( string, newArgumentList );
		}
	}
	newArgumentList->object = ctr_build_string_from_cstring( "}" );
	ctr_string_append( string, newArgumentList );
	ctr_heap_free( newArgumentList );
	return string;
}

/**
 * C-constructor function.
 *
 * This function gets called when the plugin is loaded into memory.
 * Here we have a chance to add the new object(s) to the World.
 *
 * In this case, we are going to add the Json object to the
 * world.
 */
void begin(){
	ctr_object* jsonObject = ctr_json_new(CtrStdObject, NULL);
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "parse:" ), &ctr_json_parse );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( "jsonify:" ), &ctr_json_jsonify );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "Json" ), jsonObject, CTR_CATEGORY_PUBLIC_PROPERTY);
}
