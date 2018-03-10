#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sodium.h>
#include "../../citrine.h"


/**
 * [Hash] new: [String]
 *
 * Creates a new hash from the string. 
 */
ctr_object* ctr_hash_new(ctr_object* myself, ctr_argument* argumentList) {
	char hashed_password[crypto_pwhash_STRBYTES];
	ctr_object* cryptInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	cryptInstance->link = myself;
	ctr_object* value = ctr_build_string( (char*	) &hashed_password, crypto_pwhash_STRBYTES );
	ctr_internal_object_add_property( cryptInstance, ctr_build_string_from_cstring("value"), value, CTR_CATEGORY_PRIVATE_PROPERTY );
	ctr_object* password = ctr_internal_cast2string( argumentList->object );
	if (crypto_pwhash_str(value->value.svalue->value, password->value.svalue->value, password->value.svalue->vlen,
     crypto_pwhash_OPSLIMIT_MODERATE, crypto_pwhash_MEMLIMIT_MODERATE) != 0) {
		CtrStdFlow = ctr_build_string_from_cstring("Unable to encrypt password.");
	}
	return cryptInstance;
}

ctr_object* ctr_password_verify(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* value = ctr_internal_object_find_property( myself, ctr_build_string_from_cstring("value"), CTR_CATEGORY_PRIVATE_PROPERTY );
	if (value == NULL) value = ctr_build_empty_string();
	ctr_object* compare = ctr_internal_cast2string( argumentList->object );
	if (crypto_pwhash_str_verify(value->value.svalue->value, compare->value.svalue->value, compare->value.svalue->vlen) != 0) {
		return ctr_build_bool(0);
    }
    return ctr_build_bool(1);
}

ctr_object* ctr_password_to_string(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* answer = ctr_internal_object_find_property( myself, ctr_build_string_from_cstring("value"), CTR_CATEGORY_PRIVATE_PROPERTY );
	if (answer == NULL) return CtrStdNil;
	return answer;
}

ctr_object* ctr_password_from_hash(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* cryptInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	cryptInstance->link = myself;
	ctr_internal_object_set_property( cryptInstance, ctr_build_string_from_cstring("value") , argumentList->object, CTR_CATEGORY_PRIVATE_PROPERTY );
	return cryptInstance;
}


void begin(){
	if (sodium_init() >= 0) {
		ctr_object* cryptObject = ctr_internal_create_object( CTR_OBJECT_TYPE_OTOBJECT );
		cryptObject->link = CtrStdObject;
		ctr_internal_create_func(cryptObject, ctr_build_string_from_cstring( "new:" ), &ctr_hash_new );
    	ctr_internal_create_func(cryptObject, ctr_build_string_from_cstring( "=" ), &ctr_password_verify );
    	ctr_internal_create_func(cryptObject, ctr_build_string_from_cstring( "toString" ), &ctr_password_to_string );
    	ctr_internal_create_func(cryptObject, ctr_build_string_from_cstring( "fromHash:" ), &ctr_password_from_hash );
    	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "Password" ), cryptObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	}
}
