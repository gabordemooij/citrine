#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sodium.h>
#include "../../citrine.h"

/**
 * [Password] new: [String]
 *
 * Creates a new password from the specified string of characters.
 * Upon creation, the string will be hashed immediately. Any attempt to
 * display the password will result in outputting the hash.
 *
 * Usage:
 *
 * ☞ myPassword := Password new: 'secret123'.
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

/**
 * [Password] = [String]
 *
 * Compares a string with the password. This operation will check if hash of the password equals the
 * hash of the string (and therefore if the string can be considered the correct password). Note that this
 * operation takes the form of an '=' message. So comparing passwords will always be safe. Also, under
 * the hood this message employs the hash specific time constant verification function to combat
 * timing attacks. In the example below we create a new password from string 'secret123'.
 * The moment we create the password instance, the string will already be hashed using
 * the underlying hashing algorithm provided by the current LibSodium version.
 * All information required to verify passwords will be stored in this string as well. After
 * creation of a Password object, the original input will not be retrievable anymore. To compare
 * the password using a hash specific verification function with protection against timing attacks
 * all you have to do is use the '=' message. This message implements all the logic required
 * to perform a secure password verification without burdening the developer.
 *
 * Usage:
 *
 * ☞ myPassword := Password new: 'secret123'.
 * ( myPassword = 'secret123' ) ifTrue: {
 * 	✎ write: 'You are logged in!'.
 * }.
 */
ctr_object* ctr_password_verify(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* value = ctr_internal_object_find_property( myself, ctr_build_string_from_cstring("value"), CTR_CATEGORY_PRIVATE_PROPERTY );
	if (value == NULL) value = ctr_build_empty_string();
	ctr_object* compare = ctr_internal_cast2string( argumentList->object );
	if (crypto_pwhash_str_verify(value->value.svalue->value, compare->value.svalue->value, compare->value.svalue->vlen) != 0) {
		return ctr_build_bool(0);
    }
    return ctr_build_bool(1);
}

/**
 * [Password] toString
 *
 * Returns a string representation of the password.
 * This will always return a string
 * of the hashed password. There is no way to extract the original input from the
 * Password object. This message will also be send if somehow an other object is forced
 * to perform a 'toString' conversion, like in the following example:
 *
 * Usage:
 *
 * ☞ myPassword := Password new: 'secret123'.
 * ✎ write: myPassword. #outputs hash
 */
ctr_object* ctr_password_to_string(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* answer = ctr_internal_object_find_property( myself, ctr_build_string_from_cstring("value"), CTR_CATEGORY_PRIVATE_PROPERTY );
	if (answer == NULL) return CtrStdNil;
	return answer;
}

/**
 * [Password] fromHash: [String].
 *
 * Returns a password instance representing the password described by the specified string.
 * This message can be used to load an existing password hash from the database
 * into the password object for comparison.
 */
ctr_object* ctr_password_from_hash(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* cryptInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	cryptInstance->link = myself;
	ctr_internal_object_set_property( cryptInstance, ctr_build_string_from_cstring("value") , argumentList->object, CTR_CATEGORY_PRIVATE_PROPERTY );
	return cryptInstance;
}

/**
 * @internal
 * Gets called after loading the plugin.
 * This function will add the Password object to the World.
 */
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
