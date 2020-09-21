#include <string.h>
#include <regex.h>
#include "../../citrine.h"



ctr_object* ctr_regex_new( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_object* regexInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	regexInstance->link = myself;
	ctr_internal_object_set_property(
		regexInstance, 
		ctr_build_string_from_cstring( "pattern" ),
		ctr_build_string_from_cstring( ".+" ),
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	return regexInstance;
}


ctr_object* ctr_regex_new_set( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_object* regexInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	regexInstance->link = myself;
	ctr_internal_object_set_property(
		regexInstance, 
		ctr_build_string_from_cstring( "pattern" ),
		ctr_internal_cast2string( argumentList->object ),
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	return regexInstance;
}


ctr_object* ctr_regex_find_pattern_options_do( ctr_object* myself, ctr_argument* argumentList ) {
	regex_t pattern;
	int reti;
	int sticky1, sticky2, sticky3, sticky4;
	int regex_error = 0;
	size_t n = 255;
	size_t i = 0;
	regmatch_t matches[255];
	char* needle = ctr_heap_allocate_cstring(
		ctr_internal_object_find_property(
			myself,                                    
			ctr_build_string_from_cstring( "pattern" ), 
			CTR_CATEGORY_PRIVATE_PROPERTY
		)
	);
	char* options = ctr_heap_allocate_cstring( ctr_internal_cast2string( argumentList->next->next->object ) );
	uint8_t olen = strlen( options );
	uint8_t p = 0;
	uint8_t flagIgnore = 0;
	uint8_t flagNewLine = 0;
	uint8_t flagCI = 0;
	for ( p = 0; p < olen; p ++ ) {
		if ( options[p] == '!' ) {
			flagIgnore = 1;
		}
		if ( options[p] == 'n' ) {
			flagNewLine = 1;
		}
		if ( options[p] == 'i' ) {
			flagCI = 1;
		}
	}
	ctr_object* block = argumentList->next->object;
	if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
		ctr_heap_free( needle );
		ctr_heap_free( options );
		CtrStdFlow = ctr_error("No block?",0);
		return myself;
	}
	int eflags = REG_EXTENDED;
	if (flagNewLine) eflags |= REG_NEWLINE;
	if (flagCI) eflags |= REG_ICASE;
	reti = regcomp(&pattern, needle, eflags);
	if ( reti ) {
		ctr_heap_free( needle );
		ctr_heap_free( options );
		CtrStdFlow = ctr_error("Regex error",0);
		return CtrStdNil;
	}
	char* haystack = ctr_heap_allocate_cstring(argumentList->object);
	size_t offset = 0;
	ctr_object* newString = ctr_build_empty_string();
	while( !regex_error && !flagIgnore ) {
		regex_error = regexec(&pattern, haystack + offset , n, matches, 0 );
		if ( regex_error ) break;
		ctr_argument* blockArguments;
		blockArguments = ctr_heap_allocate( sizeof( ctr_argument ) );
		ctr_argument* arrayConstructorArgument;
		arrayConstructorArgument = ctr_heap_allocate( sizeof( ctr_argument ) );
		blockArguments->object = ctr_array_new( CtrStdArray, arrayConstructorArgument );
		for( i = 0; i < n; i ++ ) {
			if ( matches[i].rm_so == -1 ) break;
			ctr_argument* group;
			group = ctr_heap_allocate( sizeof( ctr_argument ) );
			size_t len = (matches[i].rm_eo - matches[i].rm_so);
			char* tmp = ctr_heap_allocate( len + 1 );
			memcpy( tmp, haystack + offset + matches[i].rm_so, len );
			group->object = ctr_build_string_from_cstring( tmp );
			ctr_array_push( blockArguments->object, group );
			ctr_heap_free( group );
			ctr_heap_free( tmp );
		}
		if (matches[0].rm_eo != -1) {
			ctr_argument* arg = ctr_heap_allocate( sizeof( ctr_argument ) );
			arg->object = ctr_build_string( haystack + offset, matches[0].rm_so );
			ctr_string_append( newString, arg );
			offset += matches[0].rm_eo;
			ctr_heap_free( arg );
		}
		sticky1 = block->info.sticky;
		sticky2 = blockArguments->object->info.sticky;
		sticky3 = newString->info.sticky;
		sticky4 = myself->info.sticky;
		block->info.sticky = 1;
		blockArguments->object->info.sticky = 1;
		newString->info.sticky = 1;

		ctr_gc_internal_pin(block);
		ctr_gc_internal_pin(blockArguments->object);
		ctr_gc_internal_pin(newString);
		ctr_gc_internal_pin(myself);
		ctr_object* replacement = replacement = ctr_block_run( block, blockArguments, NULL );
		block->info.sticky = sticky1;
		blockArguments->object->info.sticky = sticky2;
		newString->info.sticky = sticky3;
		myself->info.sticky = sticky4;
		ctr_argument* arg = ctr_heap_allocate( sizeof( ctr_argument ) );
		arg->object = replacement;
		ctr_string_append( newString, arg );
		ctr_heap_free( arg );
		ctr_heap_free(blockArguments);
		ctr_heap_free(arrayConstructorArgument);
	}
	ctr_argument* arg = ctr_heap_allocate( sizeof( ctr_argument ) );
	arg->object = ctr_build_string( haystack + offset, strlen( haystack + offset ) );
	ctr_string_append( newString, arg );
	ctr_heap_free( arg );
	ctr_heap_free( needle );
	ctr_heap_free( haystack );
	ctr_heap_free( options );
	regfree( &pattern );
	return newString;
}

ctr_object* ctr_regex_find_pattern_do( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_argument* no_options = ctr_heap_allocate( sizeof( ctr_argument ) );
	argumentList->next->next->object = ctr_build_empty_string();
	ctr_object* answer;
	answer = ctr_regex_find_pattern_options_do( myself, argumentList );
	ctr_heap_free( no_options );
	return answer;
}


ctr_object* ctr_regex_matches( ctr_object* myself, ctr_argument* argumentList ) {
	regex_t pattern;
	int regex_error = 0;
	int result = 0;
	char* error_message = ctr_heap_allocate( 255 );
	char* needle = ctr_heap_allocate_cstring(
		ctr_internal_object_find_property(
			myself,                                    
			ctr_build_string_from_cstring( "pattern" ), 
			CTR_CATEGORY_PRIVATE_PROPERTY
		)
	);
	char* haystack = ctr_heap_allocate_cstring( ctr_internal_cast2string( argumentList->object ) );
	ctr_object* answer;
	regex_error = regcomp(&pattern, needle, REG_EXTENDED);
	if ( regex_error ) {
		CtrStdFlow = ctr_error( "Regex error", 0 );
		answer = CtrStdNil;
	} else {
		result = regexec(&pattern, haystack, 0, NULL, 0 );
		if ( !result ) {
			answer = ctr_build_bool( 1 );
		} else if ( result == REG_NOMATCH ) {
			answer = ctr_build_bool( 0 );
		} else {
			CtrStdFlow = ctr_error( error_message, 0 );
			answer = CtrStdNil;
		}
	}
	regfree( &pattern );
	ctr_heap_free( error_message );
	ctr_heap_free( needle );
	ctr_heap_free( haystack );
	return answer;
}


void begin(){
	ctr_object* regexObject = ctr_regex_new(CtrStdObject, NULL);
	regexObject->link = CtrStdObject;
	ctr_internal_create_func(regexObject, ctr_build_string_from_cstring( "new" ), &ctr_regex_new );
	ctr_internal_create_func(regexObject, ctr_build_string_from_cstring( "new:" ), &ctr_regex_new_set );
	ctr_internal_create_func(regexObject, ctr_build_string_from_cstring( "process:do:options:" ), &ctr_regex_find_pattern_options_do );
	ctr_internal_create_func(regexObject, ctr_build_string_from_cstring( "process:do:" ), &ctr_regex_find_pattern_do );
	ctr_internal_create_func(regexObject, ctr_build_string_from_cstring( "matches:" ), &ctr_regex_matches );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "Regex" ), regexObject, CTR_CATEGORY_PUBLIC_PROPERTY);
}
