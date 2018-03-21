#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>

#ifdef __MINGW32__
#include <winsock2.h>
#include <Ws2tcpip.h>
//#include <pcreposix.h>
#include <tre/regex.h>
#include "win/rand.h"

#define SHUT_RDWR SD_BOTH
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <regex.h>

#ifdef forLinux
#include <bsd/stdlib.h>
#include <bsd/string.h>
#endif
#endif

#include "citrine.h"
#include "siphash.h"

/**
 * Nil
 *
 * Nil represents 'nothing' or NULL in other languages.
 * Any object property that has not been assigned a value
 * will contain Nil. Unlike some other programming languages
 * Citrine has no concept of 'undefined' or isset, Nil is actually the
 * same as 'undefined' or not set.
 *
 * Literal:
 *
 * Nil
 */
ctr_object* ctr_build_nil() {
	return CtrStdNil;
}

/**
 * [Nil] isNil
 *
 * Nil always answers this message with a boolean object 'True'.
 */
ctr_object* ctr_nil_is_nil(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(1);
}

/**
 * [Nil] toString
 *
 * Returns the string representation of Nil: 'Nil'.
 */
ctr_object* ctr_nil_to_string(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_string_from_cstring( "Nil" );
}

/**
 * [Nil] toNumber
 *
 * Returns the numerical representation of Nil: 0.
 */
ctr_object* ctr_nil_to_number(ctr_object* myself, ctr_argument* ctr_argumentList) {
	return ctr_build_number_from_float(0);
}

/**
 * [Nil] toBoolean
 *
 * Returns the boolean representation of Nil: False.
 */
ctr_object* ctr_nil_to_boolean(ctr_object* myself, ctr_argument* ctr_argumentList) {
	return ctr_build_bool(0);
}

/**
 * Object
 *
 * This is the base object, the parent of all other objects.
 * It contains essential object oriented programming features.
 */
ctr_object* ctr_object_make(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* objectInstance = NULL;
	objectInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	objectInstance->link = myself;
	return objectInstance;
}

/**
 * [Object] type
 *
 * Returns a string representation of the type of object.
 */
ctr_object* ctr_object_type(ctr_object* myself, ctr_argument* argumentList) {
	switch(myself->info.type){
		case CTR_OBJECT_TYPE_OTNIL:
			return ctr_build_string_from_cstring("Nil");
		case CTR_OBJECT_TYPE_OTBOOL:
			return ctr_build_string_from_cstring("Boolean");
		case CTR_OBJECT_TYPE_OTNUMBER:
			return ctr_build_string_from_cstring("Number");
		case CTR_OBJECT_TYPE_OTSTRING:
			return ctr_build_string_from_cstring("String");
		case CTR_OBJECT_TYPE_OTBLOCK:
		case CTR_OBJECT_TYPE_OTNATFUNC:
			return ctr_build_string_from_cstring("Block");
		default:
			return ctr_build_string_from_cstring("Object");
	}
}

/**
 * [Object] toString
 *
 * Returns a string representation of a generic object.
 * This string representation will be:
 *
 * [Object]
 */
ctr_object* ctr_object_to_string( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_build_string_from_cstring( "[Object]" );
}

/**
 * [Object] toNumber
 *
 * Returns a numerical representation of the object. This basic behavior, part
 * of any object will just return 1. Other objects typically override this
 * behavior with more useful implementations.
 */
ctr_object* ctr_object_to_number(ctr_object* myself, ctr_argument* ctr_argumentList) {
	return ctr_build_number_from_float(1);
}

/**
 * [Object] toBoolean
 *
 * Returns a boolean representation of the object. This basic behavior, part
 * of any object will just return True. Other objects typically override this
 * behavior with more useful implementations.
 */
ctr_object* ctr_object_to_boolean(ctr_object* myself, ctr_argument* ctr_argumentList) {
	return ctr_build_bool(1);
}

/**
 * [Object] equals: [other]
 *
 * Tests whether the current instance is the same as
 * the argument.
 *
 * Alias: =
 *
 * Usage:
 * object equals: other
 */
ctr_object* ctr_object_equals(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherObject = argumentList->object;
	if (otherObject == myself) return ctr_build_bool(1);
	return ctr_build_bool(0);
}

/**
 * [Object] myself
 *
 * Returns the object itself.
 */
ctr_object* ctr_object_myself(ctr_object* myself, ctr_argument* argumentList) {
	return myself;
}

/**
 * [Object] learn: [String] means: [String].
 *
 * Teaches any object to repsond to the first specified message just like
 * it would upon receiving the second. This allows you to map existing
 * responses to new messages. You can use this to translate messages into your native
 * language. After mapping, sending the alias message will be just as fast
 * as sending the original message. You can use this to create programs
 * in your native language without sacrficing performance. Of course the mapping itself
 * has a cost, but the mapped calls will be 'toll-free'.
 *
 * Usage:
 *
 * #in this example we'll map a message to a Dutch word:
 *
 * Boolean learn: 'alsWaar:'
 *         means: 'ifTrue:'.
 *
 * (2 > 1) alsWaar: {
 *   Pen write: 'alsWaar means ifTrue in Dutch'.
 * }
 */
ctr_object* ctr_object_learn_meaning(ctr_object* myself, ctr_argument* ctr_argumentList) {
	char*  current_method_name_str;
	ctr_size     current_method_name_len;
	ctr_size     i                      = 0;
	ctr_size     len                    = 0;
	ctr_mapitem* current_method         = myself->methods->head;
	ctr_object*  target_method_name     = ctr_internal_cast2string( ctr_argumentList->next->object );
	char*        target_method_name_str = target_method_name->value.svalue->value;
	ctr_size     target_method_name_len = target_method_name->value.svalue->vlen;
	ctr_object*  alias                  = ctr_internal_cast2string( ctr_argumentList->object );
	while( i < myself->methods->size ) {
		current_method_name_str = current_method->key->value.svalue->value;
		current_method_name_len = current_method->key->value.svalue->vlen;
		if (  current_method_name_len > target_method_name_len ) {
			len = current_method_name_len;
		} else {
			len = target_method_name_len;
		}
		if ( strncmp( current_method_name_str, target_method_name_str, len ) == 0 ) {
			ctr_internal_object_add_property( myself, alias, current_method->value, 1);
			break;
		}
		current_method = current_method->next;
		i ++;
	}
	return myself;
}


/**
 * [Object] do
 *
 * Activates 'chain mode'. If chain mode is active, all messages will
 * return the recipient object regardless of their return signature.
 *
 * Usage:
 *
 * a := Array < 'hello' ; 'world' ; True ; Nil ; 666.
 * a do pop shift unshift: 'hi', push: 999, done.
 *
 * Because of 'chain mode' you can do 'a do pop shift' etc, instead of
 *
 * a pop.
 * a shift.
 * etc..
 *
 * The 'do' message tells the object to always return itself and disgard
 * the original return value until the message 'done' has been received.
 */
ctr_object* ctr_object_do( ctr_object* myself, ctr_argument* argumentList ) {
	myself->info.chainMode = 1;
	return myself;
}

/**
 * [Object] done
 *
 * Deactivates 'chain mode'.
 */
ctr_object* ctr_object_done( ctr_object* myself, ctr_argument* argumentList ) {
	myself->info.chainMode = 0;
	return myself;
}

/**
 * [Object] case: [Object] do: [Block].
 *
 * This message makes the recipient compare itself to the specified object.
 * If the recipient considers itself to be equal, it will carry out the
 * instructions in the associated block of code. The recipient will send
 * the message '=' to itself with the other object as an argument. This leaves
 * it up to the recipient to determine whether the objects are considered
 * equal. If the recipient decides the objects are not equal, the associated
 * code block will be ignored. Note that this allows you to implement a
 * so-called switch-statement like those found in other languages. Because
 * of the generic implementation, you can use the case statements on almost
 * any object. Case-do statements may provide a readable alternative to a
 * long list of if-else messages.
 *
 * The example program below will print the text 'It's a Merlot!'.
 *
 * Usage:
 *
 * #Can we use a switch statement in Citrine?
 *
 * #create a good wine
 * ☞ wine := Object new.
 *
 * #define a string representation for this wine
 * wine on: 'toString' do: {
 *	↲ 'merlot'.
 * }.
 *
 * #define how wines are to be compared
 * wine on: '=' do: { :other wine
 *	↲ ( me toString = other wine ).
 *}.
 *
 * #now select the correct wine from the list
 * wine
 *	case: 'cabernet' do: { ✎ write: 'it\'s a Cabernet!'. },
 *	case: 'syrah'    do: { ✎ write: 'it\'s a Syrah!'.    },
 *	case: 'merlot'   do: { ✎ write: 'it\'s a Merlot!'.   },
 *	case: 'malbec'   do: { ✎ write: 'it\'s a Malbec!'.   }.
 */
ctr_object* ctr_object_case_do( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_object* block = argumentList->next->object;
	ctr_argument* compareArguments;
	if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
		CtrStdFlow = ctr_build_string_from_cstring("Expected block for case.");
		return CtrStdNil;
	}
	compareArguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	compareArguments->object = argumentList->object;
	compareArguments->next = NULL;
	if (
		ctr_internal_cast2bool(
			ctr_send_message( myself, "=", 1, compareArguments )
		)->value.bvalue == 1) {
		block->info.sticky = 1;
		ctr_block_run(block, compareArguments, NULL);
		if (CtrStdFlow == CtrStdContinue) CtrStdFlow = NULL; /* consume continue */
		if (CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL; /* consume break */
		block->info.mark = 0;
		block->info.sticky = 0;
	}
	ctr_heap_free( compareArguments );
	return myself;
}

/**
 * [Object] message: [String] arguments: [Array]
 *
 * Sends a custom or 'dynamic' message to an object. This takes a string containing
 * the message to be send to the object and an array listing the arguments at the
 * correct indexes. If the array fails to provide the correct indexes this will
 * generate an out-of-bounds error coming from the Array object. If something other
 * than an Array is provided an error will be thrown as well.
 *
 * Usage:
 *
 * var str := 'write:'.
 * Pen message: 'write:' arguments: (Array < 'Hello World').
 *
 * This will print the string 'Hello world' on the screen using a dynamically
 * crafted message.
 */
ctr_object* ctr_object_message( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_object* message = ctr_internal_cast2string( argumentList->object );
	ctr_object* arr     = argumentList->next->object;
	if ( arr->info.type != CTR_OBJECT_TYPE_OTARRAY ) {
		ctr_error_text( "Dynamic message expects array." );
		return CtrStdNil;
	}
	ctr_size length = (int) ctr_array_count( arr,  NULL )->value.nvalue;
	int i = 0;
	ctr_argument* args = ctr_heap_allocate( sizeof( ctr_argument ) );
	ctr_argument* cur  = args;
	for ( i = 0; i < length; i ++ ) {
		ctr_argument* index = ctr_heap_allocate( sizeof( ctr_argument ) );
		if ( i > 0 ) {
			cur->next = ctr_heap_allocate( sizeof( ctr_argument ) );
			cur = cur->next;
		}
		index->object = ctr_build_number_from_float( (double) i );
		cur->object = ctr_array_get( arr, index );
		ctr_heap_free( index );
	}
	char* flatMessage = ctr_heap_allocate_cstring( message );
	ctr_object* answer = ctr_send_message( myself, flatMessage, message->value.svalue->vlen, args);
	cur = args;
	if ( length == 0 ) {
		ctr_heap_free(args);
	} else {
		for ( i = 0; i < length; i ++ ) {
			ctr_argument* a = cur;
			if ( i < length - 1 ) cur = cur->next;
			ctr_heap_free( a );
		}
	}
	ctr_heap_free( flatMessage );
	return answer;
}

/**
 * [Object] on: [String] do: [Block]
 *
 * Makes the object respond to a new kind of message.
 * Use the semicolons to indicate the positions of the arguments to be
 * passed.
 *
 * Usage:
 *
 * object on: 'greet' do: { ... }.
 * object on: 'between:and:' do: { ... }.
 *
 */
ctr_object* ctr_object_on_do(ctr_object* myself, ctr_argument* argumentList) {
	ctr_argument* nextArgument;
	ctr_object* methodBlock;
	ctr_object* methodName = argumentList->object;
	if (methodName->info.type != CTR_OBJECT_TYPE_OTSTRING) {
		CtrStdFlow = ctr_build_string_from_cstring("Expected on: argument to be of type string.");
		CtrStdFlow->info.sticky = 1;
		return myself;
	}
	nextArgument = argumentList->next;
	methodBlock = nextArgument->object;
	if (methodBlock->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
		CtrStdFlow = ctr_build_string_from_cstring("Expected argument do: to be of type block.");
		CtrStdFlow->info.sticky = 1;
		return myself;
	}
	ctr_internal_object_add_property(myself, methodName, methodBlock, 1);
	return myself;
}

/**
 * @internal
 */
ctr_object* ctr_sock_error( int fd, int want2close ) {
	CtrStdFlow = ctr_build_string_from_cstring( strerror( errno ) );
	if (want2close) {
		shutdown(fd, SHUT_RDWR);
		#ifdef __MINGW32__
		closesocket(fd);
		#else
		close(fd);
		#endif
	}
	return CtrStdNil;
}

/**
 * @internal
 */
ctr_object* ctr_object_send2remote(ctr_object* myself, ctr_argument* argumentList) {
	char* ip;
	int sockfd = 0, n = 0;
	char* responseBuff;
	size_t responseLength;
	struct sockaddr_in6 serv_addr;
	ctr_object* answer;
	ctr_object* messageObj;
	ctr_object* ipObj;
	struct hostent *server;
	ipObj = ctr_internal_object_find_property(
		myself,
		ctr_build_string_from_cstring("@"),
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	if (ipObj == NULL) return CtrStdNil;
	ipObj = ctr_internal_cast2string(ipObj);
	ip = ctr_heap_allocate_cstring(ipObj);
	answer = ctr_build_empty_string();
	messageObj = ctr_internal_cast2string(
		argumentList->object
	);
	if((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) return ctr_sock_error( sockfd, 0 );
	memset(&serv_addr, '0', sizeof(serv_addr));
	#ifdef __MINGW32__
	server = gethostbyname(ip);
	#else
	server = gethostbyname2(ip,AF_INET6);
	#endif
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin6_flowinfo = 0;
    serv_addr.sin6_family = AF_INET6;
    memmove((char *) &serv_addr.sin6_addr.s6_addr, (char *) server->h_addr, server->h_length);
    serv_addr.sin6_port = htons(ctr_default_port);
	int c = 0;
	c = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if ( c != 0 ) return ctr_sock_error( sockfd, 1 );
	c = send(sockfd, (char*) &messageObj->value.svalue->vlen, sizeof(size_t), 0);
	if ( c < 0 ) ctr_sock_error( sockfd, 1 );
	c = send(sockfd, messageObj->value.svalue->value, messageObj->value.svalue->vlen, 0);
	if ( c < 0 ) ctr_sock_error( sockfd, 1 );
	#ifdef __MINGW32__
	n = recv(sockfd, (char*) &responseLength, sizeof(responseLength), 0);
	#else	
	n = read(sockfd, (size_t*) &responseLength, sizeof(responseLength));
	#endif
	if ( n == 0 ) ctr_sock_error( sockfd, 1 );
	responseBuff = ctr_heap_allocate( responseLength + 1 );
	#ifdef __MINGW32__
	n = recv(sockfd, responseBuff, responseLength, 0);
	#else
	n = read(sockfd, responseBuff, responseLength);
	#endif
	if ( n == 0 ) ctr_sock_error( sockfd, 1 );
	answer = ctr_build_string_from_cstring( responseBuff );
	shutdown(sockfd, SHUT_RDWR);
	#ifdef __MINGW32__
	closesocket(sockfd);
	#else
	close(sockfd);
	#endif
	ctr_heap_free(ip);
	ctr_heap_free(responseBuff);
	return answer;
}

/**
 * [Object] respondTo: [String]
 *
 * Variations:
 *
 * [Object] respondTo: [String] with: [String]
 * [Object] respondTo: [String] with: [String] and: [String]
 *
 * Default respond-to implemention, does nothing.
 * You can override this behaviour to implement generic behaviour.
 * Listening to these messages allows users to send any message to an
 * object. For instance an object can respond to any message it does not
 * understand by echoing the message.
 */
ctr_object* ctr_object_respond(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->info.remote == 0) return myself;
	ctr_object* arr;
	ctr_object* answer;
	ctr_argument* newArgumentList;
	arr = ctr_array_new( CtrStdArray, argumentList );
	newArgumentList = ctr_heap_allocate( sizeof(ctr_argument) );
	newArgumentList->object = argumentList->object;
	ctr_array_push( arr, newArgumentList );
	newArgumentList->object = ctr_array_to_string( arr, NULL );
	answer = ctr_object_send2remote( myself, newArgumentList );
	ctr_heap_free(newArgumentList);
	return answer;
}


ctr_object* ctr_object_respond_and(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->info.remote == 0) return myself;
	ctr_object* arr;
	ctr_object* answer;
	ctr_argument* newArgumentList;
	arr = ctr_array_new( CtrStdArray, argumentList );
	newArgumentList = ctr_heap_allocate( sizeof(ctr_argument) );
	newArgumentList->object = argumentList->object;
	ctr_array_push( arr, newArgumentList );
	newArgumentList->object = argumentList->next->object;
	ctr_array_push( arr, newArgumentList );
	newArgumentList->object = ctr_array_to_string( arr, NULL );
	answer = ctr_object_send2remote( myself, newArgumentList );
	ctr_heap_free(newArgumentList);
	return answer;
}

ctr_object* ctr_object_respond_and_and(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->info.remote == 0) return myself;
	ctr_object* arr;
	ctr_object* answer;
	ctr_argument* newArgumentList;
	arr = ctr_array_new( CtrStdArray, argumentList );
	newArgumentList = ctr_heap_allocate( sizeof(ctr_argument) );
	newArgumentList->object = argumentList->object;
	ctr_array_push( arr, newArgumentList );
	newArgumentList->object = argumentList->next->object;
	ctr_array_push( arr, newArgumentList );
	newArgumentList->object = argumentList->next->next->object;
	ctr_array_push( arr, newArgumentList );
	newArgumentList->object = ctr_array_to_string( arr, NULL );
	answer = ctr_object_send2remote( myself, newArgumentList );
	ctr_heap_free(newArgumentList);
	return answer;
}

/**
 * [Object] isNil
 *
 * Default isNil implementation.
 *
 * Always returns boolean object False.
 */
ctr_object* ctr_object_is_nil(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(0);
}

/**
 * Boolean
 *
 * Literal:
 *
 * True
 * False
 */
ctr_object* ctr_build_bool(int truth) {
	ctr_object* boolObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTBOOL);
	if (truth) boolObject->value.bvalue = 1; else boolObject->value.bvalue = 0;
	boolObject->info.type = CTR_OBJECT_TYPE_OTBOOL;
	boolObject->link = CtrStdBool;
	return boolObject;
}

/**
 * [Boolean] = [other]
 *
 * Tests whether the other object (as a boolean) has the
 * same value (boolean state True or False) as the current one.
 *
 * Usage:
 *
 * (True = False) ifFalse: { Pen write: 'This is not True!'. }.
 */
ctr_object* ctr_bool_eq(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(ctr_internal_cast2bool(argumentList->object)->value.bvalue == myself->value.bvalue);
}


/**
 * [Boolean] != [other]
 *
 * Tests whether the other object (as a boolean) has the
 * same value (boolean state True or False) as the current one.
 *
 * Usage:
 *
 * (True != False) ifTrue: { Pen write: 'This is not True!'. }.
 */
ctr_object* ctr_bool_neq(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(ctr_internal_cast2bool(argumentList->object)->value.bvalue != myself->value.bvalue);
}

/**
 * [Boolean] toString
 *
 * Simple cast function.
 */
ctr_object* ctr_bool_to_string(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.bvalue == 1) {
		return ctr_build_string_from_cstring( "True" );
	} else {
		return ctr_build_string_from_cstring( "False" );
	}
}

/**
 * [Boolean] break
 *
 * Breaks out of the current block and bubbles up to the parent block if
 * the value of the receiver equals boolean True.
 *
 * Usage:
 *
 * (iteration > 10) break. #breaks out of loop after 10 iterations
 */
ctr_object* ctr_bool_break(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.bvalue) {
		CtrStdFlow = CtrStdBreak; /* If error = Break it's a break, there is no real error. */
	}
	return myself;
}

/**
 * [Boolean] continue
 *
 * Skips the remainder of the current block in a loop, continues to the next
 * iteration.
 *
 * Usage:
 *
 * (iteration > 10) continue.
 */
ctr_object* ctr_bool_continue(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.bvalue) {
		CtrStdFlow = CtrStdContinue; /* If error = Continue, then it breaks only one iteration (return). */
	}
	return myself;
}

/**
 * [Boolean] ifTrue: [block]
 *
 * Executes a block of code if the value of the boolean
 * object is True.
 *
 * Usage:
 * (some expression) ifTrue: { ... }.
 *
 * You can also use ifFalse and ifTrue with other objects because the
 * Object instance also responds to these messages.
 */
ctr_object* ctr_bool_if_true(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* result;
	if (myself->value.bvalue) {
		ctr_object* codeBlock = argumentList->object;
		ctr_argument* arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		arguments->object = myself;
		result = ctr_block_run(codeBlock, arguments, NULL);
		ctr_heap_free( arguments );
		return result;
	}
	if (CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL; /* consume break */
	return myself;
}

/**
 * [Boolean] ifFalse: [block]
 *
 * Executes a block of code if the value of the boolean
 * object is True.
 *
 * Usage:
 * (some expression) ifFalse: { ... }.
 *
 * You can also use ifFalse and ifTrue with other objects because the
 * Object instance also responds to these messages.
 */
ctr_object* ctr_bool_if_false(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* result;
	if (!myself->value.bvalue) {
		ctr_object* codeBlock = argumentList->object;
		ctr_argument* arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		arguments->object = myself;
		result = ctr_block_run(codeBlock, arguments, NULL);
		ctr_heap_free( arguments );
		return result;
	}
	if (CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL; /* consume break */
	return myself;
}

/**
 * @internal
 */
ctr_object* ctr_object_if_false( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_bool_if_false( ctr_internal_cast2bool( myself ), argumentList );
}

/**
 * @internal
 */
ctr_object* ctr_object_if_true( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_bool_if_true( ctr_internal_cast2bool( myself ), argumentList );
}

/**
 * [Boolean] not
 *
 * Returns the opposite of the current value.
 *
 * Usage:
 * True := False not.
 *
 */
ctr_object* ctr_bool_not(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(!myself->value.bvalue);
}

/**
 * [Boolean] either: [this] or: [that]
 *
 * Returns argument #1 if boolean value is True and argument #2 otherwise.
 *
 * Usage:
 * Pen write: 'the coin lands on: ' + (Boolean flip either: 'head' or: 'tail').
 */
ctr_object* ctr_bool_either_or(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.bvalue) {
		return argumentList->object;
	} else {
		return argumentList->next->object;
	}
}

/**
 * [Boolean] & [other]
 *
 * Returns True if both the object value is True and the
 * argument is True as well.
 *
 * Usage:
 *
 * a & b
 *
 */
ctr_object* ctr_bool_and(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* other = ctr_internal_cast2bool(argumentList->object);
	return ctr_build_bool((myself->value.bvalue && other->value.bvalue));
}

/**
 * [Boolean] ! [other]
 *
 * Returns True if the object value is False and the
 * argument is False as well.
 *
 * Usage:
 *
 * a ! b
 *
 */
ctr_object* ctr_bool_nor(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* other = ctr_internal_cast2bool(argumentList->object);
	return ctr_build_bool((!myself->value.bvalue && !other->value.bvalue));
}

/**
 * [Boolean] | [other]
 *
 * Returns True if either the object value is True or the
 * argument is True or both are True.
 *
 * Usage:
 *
 * a | b
 */
ctr_object* ctr_bool_or(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* other = ctr_internal_cast2bool(argumentList->object);
	return ctr_build_bool((myself->value.bvalue || other->value.bvalue));
}

/**
 * [Boolean] ? [other]
 *
 * Returns True if either the object value is True or the
 * argument is True but not both.
 *
 * Usage:
 *
 * a ? b
 */
ctr_object* ctr_bool_xor(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* other = ctr_internal_cast2bool(argumentList->object);
	return ctr_build_bool((myself->value.bvalue ^ other->value.bvalue));
}

/**
 * [Boolean] toNumber
 *
 * Returns 0 if boolean is False and 1 otherwise.
 */
ctr_object* ctr_bool_to_number(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float( (ctr_number) myself->value.bvalue );
}

/**
 * Number
 *
 * Literal:
 *
 * 0
 * 1
 * -8
 * 2.5
 * pi
 * 𝛑
 *
 * Represents a number object in Citrine.
 */
ctr_object* ctr_build_number(char* n) {
	ctr_object* numberObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNUMBER);
	numberObject->value.nvalue = atof(n);
	numberObject->link = CtrStdNumber;
	return numberObject;
}

/**
 * @internal
 * BuildNumberFromString
 */
ctr_object* ctr_build_number_from_string(char* str, ctr_size length) {
	char* numCStr;
	ctr_object* numberObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNUMBER);
	/* turn string into a C-string before feeding it to atof */
	int stringNumberLength = ( length <= 40 ) ? length : 40;
	/* max length is 40 (and that's probably even too long... ) */
	numCStr = (char*) ctr_heap_allocate( 41 * sizeof( char ) );
	memcpy( numCStr, str, stringNumberLength );
	numberObject->value.nvalue = atof(numCStr);
	numberObject->link = CtrStdNumber;
	ctr_heap_free( numCStr );
	return numberObject;
}

/**
 * @internal
 * BuildNumberFromFloat
 *
 * Creates a number object from a float.
 * Internal use only.
 */
ctr_object* ctr_build_number_from_float(ctr_number f) {
	ctr_object* numberObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTNUMBER);
	numberObject->value.nvalue = f;
	numberObject->link = CtrStdNumber;
	return numberObject;
}

/**
 * [Number] > [other]
 *
 * Returns True if the number is higher than other number.
 */
ctr_object* ctr_number_higherThan(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue > otherNum->value.nvalue));
}

/**
 * [Number] >=: [other]
 *
 * Returns True if the number is higher than or equal to other number.
 */
ctr_object* ctr_number_higherEqThan(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue >= otherNum->value.nvalue));
}

/**
 * [Number] < [other]
 *
 * Returns True if the number is less than other number.
 */
ctr_object* ctr_number_lowerThan(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue < otherNum->value.nvalue));
}

/**
 * [Number] <=: [other]
 *
 * Returns True if the number is less than or equal to other number.
 */
ctr_object* ctr_number_lowerEqThan(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool((myself->value.nvalue <= otherNum->value.nvalue));
}

/**
 * [Number] = [other]
 *
 * Returns True if the number equals the other number.
 */
ctr_object* ctr_number_eq(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool(myself->value.nvalue == otherNum->value.nvalue);
}

/**
 * [Number] !=: [other]
 *
 * Returns True if the number does not equal the other number.
 */
ctr_object* ctr_number_neq(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	return ctr_build_bool(myself->value.nvalue != otherNum->value.nvalue);
}

/**
 * [Number] between: [low] and: [high]
 *
 * Returns True if the number instance has a value between the two
 * specified values.
 *
 * Usage:
 *
 * q between: x and: y
 */
ctr_object* ctr_number_between(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_argument* nextArgumentItem = argumentList->next;
	ctr_object* nextArgument = ctr_internal_cast2number(nextArgumentItem->object);
	return ctr_build_bool((myself->value.nvalue >=  otherNum->value.nvalue) && (myself->value.nvalue <= nextArgument->value.nvalue));
}

/**
 * [Number] odd
 *
 * Returns True if the number is odd and False otherwise.
 */
ctr_object* ctr_number_odd(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool((int)myself->value.nvalue % 2);
}

/**
 * [Number] even
 *
 * Returns True if the number is even and False otherwise.
 */
ctr_object* ctr_number_even(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(!((int)myself->value.nvalue % 2));
}

/**
 * [Number] + [Number]
 *
 * Adds the other number to the current one. Returns a new
 * number object.
 */
ctr_object* ctr_number_add(ctr_object* myself, ctr_argument* argumentList) {
	ctr_argument* newArg;
	ctr_object* otherNum = argumentList->object;
	ctr_object* result;
	ctr_number a;
	ctr_number b;
	ctr_object* strObject;
	if (otherNum->info.type == CTR_OBJECT_TYPE_OTSTRING) {
		strObject = ctr_internal_cast2string(myself);
		newArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		newArg->object = otherNum;
		result = ctr_string_concat(strObject, newArg);
		ctr_heap_free( newArg );
		return result;
	} else {
		otherNum = ctr_internal_cast2number( otherNum );
	}
	a = myself->value.nvalue;
	b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a+b));
}

/**
 * [Number] +=: [Number]
 *
 * Increases the number ITSELF by the specified amount, this message will change the
 * value of the number object itself instead of returning a new number.
 */
ctr_object* ctr_number_inc(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	myself->value.nvalue += otherNum->value.nvalue;
	return myself;
}

/**
 * [Number] - [Number]
 *
 * Subtracts the other number from the current one. Returns a new
 * number object.
 */
ctr_object* ctr_number_minus(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_number a = myself->value.nvalue;
	ctr_number b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a-b));
}

/**
 * [Number] -=: [number]
 *
 * Decreases the number ITSELF by the specified amount, this message will change the
 * value of the number object itself instead of returning a new number.
 */
ctr_object* ctr_number_dec(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	myself->value.nvalue -= otherNum->value.nvalue;
	return myself;
}

/**
 * [Number] * [Number or Block]
 *
 * Multiplies the number by the specified multiplier. Returns a new
 * number object.
 */
ctr_object* ctr_number_multiply(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum;
	ctr_number a;
	ctr_number b;
	otherNum = ctr_internal_cast2number(argumentList->object);
	a = myself->value.nvalue;
	b = otherNum->value.nvalue;
	return ctr_build_number_from_float(a*b);
}

/**
 * [Number] times: [Block]
 *
 * Runs the block of code a 'Number' of times.
 * This is the most basic form of a loop.
 *
 * Usage:
 *
 * 7 times: { :i Pen write: i. }.
 *
 * The example above runs the block 7 times. The current iteration
 * number is passed to the block as a parameter (i in this example).
 */
ctr_object* ctr_number_times(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* indexNumber;
	ctr_object* block = argumentList->object;
	ctr_argument* arguments;
	int t;
	int i;
	if (block->info.type != CTR_OBJECT_TYPE_OTBLOCK) { fprintf(stderr, "Expected code block."); exit(1); }
	block->info.sticky = 1;
	t = myself->value.nvalue;
	arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	for(i=0; i<t; i++) {
		indexNumber = ctr_build_number_from_float((ctr_number) i);
		arguments->object = indexNumber;
		ctr_block_run(block, arguments, NULL);
		if (CtrStdFlow == CtrStdContinue) CtrStdFlow = NULL; /* consume continue */
		if (CtrStdFlow) break;
	}
	ctr_heap_free( arguments );
	if (CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL; /* consume break */
	block->info.mark = 0;
	block->info.sticky = 0;
	return myself;
}

/**
 * [Number] *=: [Number]
 *
 * Multiplies the number ITSELF by multiplier, this message will change the
 * value of the number object itself instead of returning a new number.
 *
 * Usage:
 *
 * x := 5.
 * x *=: 2. #x is now 10.
 *
 * Use this message to apply the operation to the object itself instead
 * of creating and returning a new object.
 */
ctr_object* ctr_number_mul(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	myself->value.nvalue *= otherNum->value.nvalue;
	return myself;
}

/**
 * [Number] / [Number]
 *
 * Divides the number by the specified divider. Returns a new
 * number object.
 */
ctr_object* ctr_number_divide(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_number a = myself->value.nvalue;
	ctr_number b = otherNum->value.nvalue;
	if (b == 0) {
		CtrStdFlow = ctr_build_string_from_cstring("Division by zero.");
		CtrStdFlow->info.sticky = 1;
		return myself;
	}
	return ctr_build_number_from_float((a/b));
}

/**
 * [Number] /=: [Number]
 *
 * Divides the number ITSELF by divider, this message will change the
 * value of the number object itself instead of returning a new number.
 *
 * Usage:
 *
 * x := 10.
 * x /=: 2. #x will now be 5.
 *
 * Use this message to apply the operation to the object itself instead
 * of generating a new object.
 */
ctr_object* ctr_number_div(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	if (otherNum->value.nvalue == 0) {
		CtrStdFlow = ctr_build_string_from_cstring("Division by zero.");
		return myself;
	}
	myself->value.nvalue /= otherNum->value.nvalue;
	return myself;
}

/**
 * [Number] % [modulo]
 *
 * Returns the modulo of the number. This message will return a new
 * object representing the modulo of the recipient.
 *
 * Usage:
 *
 * x := 11 % 3. #x will now be 2
 *
 * Use this message to apply the operation of division to the
 * object itself instead of generating a new one.
 */
ctr_object* ctr_number_modulo(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_number a = myself->value.nvalue;
	ctr_number b = otherNum->value.nvalue;
	if (b == 0) {
		CtrStdFlow = ctr_build_string_from_cstring("Division by zero.");
		return myself;
	}
	return ctr_build_number_from_float(fmod(a,b));
}

/**
 * [Number] toPowerOf: [power]
 *
 * Returns a new object representing the
 * number to the specified power.
 *
 * Usage:
 *
 * x := 2 toPowerOf: 8. #x will be 256
 *
 * The example above will raise 2 to the power of 8 resulting in
 * a new Number object: 256.
 */
ctr_object* ctr_number_pow(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_number a = myself->value.nvalue;
	ctr_number b = otherNum->value.nvalue;
	return ctr_build_number_from_float(pow(a,b));
}

/**
 * [Number] pos
 *
 * Returns a boolean indicating wether the number is positive.
 * This message will return a boolean object 'True' if the recipient is
 * positive and 'False' otherwise.
 *
 * Usage:
 *
 * hope := 0.1.
 * ( hope pos ) ifTrue: { Pen write: 'Still a little hope for humanity'. }.
 *
 * The example above will print the message because hope is higher than 0.
 */
ctr_object* ctr_number_positive(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool( ( myself->value.nvalue > 0) );
}

/**
 * [Number] neg
 *
 * Returns a boolean indicating wether the number is negative.
 * This message will return a boolean object 'True' if the recipient is
 * negative and 'False' otherwise. It's the eaxct opposite of the 'positive'
 * message.
 *
 * Usage:
 *
 * hope := -1.
 * (hope neg) ifTrue: { Pen write: 'No hope left'. }.
 *
 * The example above will print the message because the value of the variable
 * hope is less than 0.
 */
ctr_object* ctr_number_negative(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool( ( myself->value.nvalue < 0) );
}

/**
 * [Number] max: [other]
 *
 * Returns the biggest number of the two.
 *
 * Usage:
 *
 * x := 6 max: 4. #x is 6
 * x := 6 max: 7. #x is 7
 */
ctr_object* ctr_number_max(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_number a = myself->value.nvalue;
	ctr_number b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a >= b) ? a : b);
}

/**
 * [Number] min: [other]
 *
 * Returns a the smallest number.
 *
 * Usage:
 *
 * x := 6 min: 4. #x is 4
 * x := 6 min: 7. #x is 7
 */
ctr_object* ctr_number_min(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* otherNum = ctr_internal_cast2number(argumentList->object);
	ctr_number a = myself->value.nvalue;
	ctr_number b = otherNum->value.nvalue;
	return ctr_build_number_from_float((a <= b) ? a : b);
}

/**
 * [Number] factorial
 *
 * Calculates the factorial of a number.
 */
ctr_object* ctr_number_factorial(ctr_object* myself, ctr_argument* argumentList) {
	ctr_number t = myself->value.nvalue;
	int i;
	ctr_number a = 1;
	for(i = (int) t; i > 0; i--) {
		a = a * i;
	}
	return ctr_build_number_from_float(a);
}

/**
 * [Number] to: [number] step: [step] do: [block]
 *
 * Runs the specified block for each step it takes to go from
 * the start value to the target value using the specified step size.
 * This is basically how you write for-loops in Citrine.
 *
 * Usage:
 *
 * 1 to: 5 step: 1 do: { :step Pen write: 'this is step #'+step. }.
 */
ctr_object* ctr_number_to_step_do(ctr_object* myself, ctr_argument* argumentList) {
	double startValue = myself->value.nvalue;
	double endValue   = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	double incValue   = ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
	double curValue   = startValue;
	ctr_object* codeBlock = argumentList->next->next->object;
	ctr_argument* arguments;
	int forward = 0;
	if (startValue == endValue) return myself;
	forward = (startValue < endValue);
	if (codeBlock->info.type != CTR_OBJECT_TYPE_OTBLOCK) {
		CtrStdFlow = ctr_build_string_from_cstring("Expected block.");
		return myself;
	}
	while(((forward && curValue <= endValue) || (!forward && curValue >= endValue)) && !CtrStdFlow) {
		arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		arguments->object = ctr_build_number_from_float(curValue);
		ctr_block_run(codeBlock, arguments, NULL);
		ctr_heap_free( arguments );
		if (CtrStdFlow == CtrStdContinue) CtrStdFlow = NULL; /* consume continue and go on */
		curValue += incValue;
	}
	if (CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL; /* consume break */
	return myself;
}

/**
 * [Number] floor
 *
 * Gives the largest integer less than the recipient.
 *
 * Usage:
 *
 * x := 4.5
 * y := x floor. #y will be 4
 *
 * The example above applies the floor function to the recipient (4.5)
 * returning a new number object (4).
 */
ctr_object* ctr_number_floor(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(floor(myself->value.nvalue));
}

/**
 * [Number] qualify: 'meters'.
 *
 * Qualifies a number. Alias for: [Number] [String].
 * See the [Number] [String] message signature for more details.
 */

/**
 * [Number] [String]
 *
 * Qualifies a number. By sending an arbitrary (undocumented) unary message to
 * a Number object your message will be set as the qualification property of the number
 * and passed around along with the number value itself.
 *
 * Usage:
 *
 * Number learn: 'plus:' means: '+'.
 * Number on: '+' do: { :x
 *	☞ rate := 1.
 *	☞ currency := x qualification.
 *	(currency = 'euros') ifTrue: {
 *		rate := 2.
 *	}.
 *	↲ (⛏ plus: (x * rate)).
 * }.
 * ☞ money := 3 dollars + 2 euros. #7
 */
ctr_object* ctr_number_qualify(ctr_object* myself, ctr_argument* argumentList) {
	ctr_internal_object_set_property( myself, ctr_build_string_from_cstring( CTR_DICT_QUALIFICATION ), ctr_internal_cast2string( argumentList->object ), CTR_CATEGORY_PRIVATE_PROPERTY );
	return myself;
}

/**
 * [Number] qualification.
 *
 * Returns the qualification of a number object. For instance, as a
 * number (let's say 99) has been qualified as the number of bottles using
 * a message like: '99 bottles' this message will return the descriptive string
 * 'bottles'. For usage examples, please consult the [Number] [String] message
 * signature.
 */
ctr_object* ctr_number_qualification(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* answer = ctr_internal_object_find_property( myself, ctr_build_string_from_cstring( CTR_DICT_QUALIFICATION ), CTR_CATEGORY_PRIVATE_PROPERTY );
	if ( answer == NULL ) return CtrStdNil;
	return answer;
}

/**
 * @internal
 */
ctr_object* ctr_number_respond_to(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_number_qualify( myself, argumentList );
}

/**
 * [Number] ceil
 *
 * Rounds up the recipient number and returns the next higher integer number
 * as a result.
 *
 * Usage:
 *
 * x := 4.5.
 * y = x ceil. #y will be 5
 *
 * The example above applies the ceiling function to the recipient (4.5)
 * returning a new number object (5).
 */
ctr_object* ctr_number_ceil(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(ceil(myself->value.nvalue));
}

/**
 * [Number] round
 *
 * Returns the rounded number.
 */
ctr_object* ctr_number_round(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(round(myself->value.nvalue));
}

/**
 * [Number] abs
 *
 * Returns the absolute (unsigned, positive) value of the number.
 *
 * Usage:
 *
 * x := -7.
 * y := x abs. #y will be 7
 *
 * The example above strips the sign off the value -7 resulting
 * in 7.
 */
ctr_object* ctr_number_abs(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(fabs(myself->value.nvalue));
}

/**
 * [Number] sqrt
 *
 * Returns the square root of the recipient.
 *
 * Usage:
 *
 * x := 49.
 * y := x sqrt. #y will be 7
 *
 * The example above takes the square root of 49, resulting in the
 * number 7.
 */
ctr_object* ctr_number_sqrt(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(sqrt(myself->value.nvalue));
}

/**
 * [Number] exp
 *
 * Returns the exponent of the number.
 */
ctr_object* ctr_number_exp(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(exp(myself->value.nvalue));
}

/**
 * [Number] sin
 *
 * Returns the sine of the number.
 */

ctr_object* ctr_number_sin(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(sin(myself->value.nvalue));
}

/**
 * [Number] cos
 *
 * Returns the cosine of the number.
 */
ctr_object* ctr_number_cos(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(cos(myself->value.nvalue));
}

/**
 * [Number] tan
 *
 * Caculates the tangent of a number.
 */
ctr_object* ctr_number_tan(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(tan(myself->value.nvalue));
}

/**
 * [Number] atan
 *
 * Caculates the arctangent of a number.
 */
ctr_object* ctr_number_atan(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(atan(myself->value.nvalue));
}

/**
 * [Number] log
 *
 * Calculates the logarithm of a number.
 */
ctr_object* ctr_number_log(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float(log(myself->value.nvalue));
}

/**
 * [Number] toByte
 *
 * Converts a number to a single byte.
 */
ctr_object* ctr_number_to_byte(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* str = ctr_build_empty_string();
	str->value.svalue->value = ctr_heap_allocate( 1 );
	str->value.svalue->vlen = 1;
	*(str->value.svalue->value) = (uint8_t) myself->value.nvalue;
	return str;
}

/**
 * [Number] toString
 *
 * Wrapper for cast function.
 */
ctr_object* ctr_number_to_string(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* o = myself;
	int slen;
	char* s;
	char* p;
	char* buf;
	int bufSize;
	ctr_object* stringObject;
	s = ctr_heap_allocate( 80 * sizeof( char ) );
	bufSize = 100 * sizeof( char );
	buf = ctr_heap_allocate( bufSize );
	snprintf( buf, 99, "%.10f", o->value.nvalue );
	p = buf + strlen(buf) - 1;
	while ( *p == '0' && *p-- != '.' );
	*( p + 1 ) = '\0';
	if ( *p == '.' ) *p = '\0';
	strncpy( s, buf, strlen( buf ) );
	ctr_heap_free( buf );
	slen = strlen(s);
	stringObject = ctr_build_string(s, slen);
	ctr_heap_free( s );
	return stringObject;
}

/**
 * [Number] toBoolean
 *
 * Casts a number to a boolean object.
 */
ctr_object* ctr_number_to_boolean(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool( myself->value.nvalue );
}

/**
 * String
 *
 * Literal:
 *
 * 'Hello World, this is a String.'
 *
 * A sequence of bytes or characters. In Citrine, strings are UTF-8 aware.
 * You may only use single quotes. To escape a character use the
 * backslash '\' character. Use the special characters ↵ and ⇿ to
 * insert a newline or tab respectively.
 *
 * The following string constants exist:
 *
 * a-z equals abcdefghijklmnopqrstuvwxyz
 * A-Z equals ABCDEFGHIJKLMNOPQRESTUVWXYZ
 *
 * Strings in Citrine represent a series of bytes. Strings can be
 * interpreted as real bytes or as text depending on the messages
 * send. For instance, the message 'bytes' returns the number of bytes
 * in a string, while the message 'length' returns the number of
 * characters (as defined as separate UTF-8 code points) in a string.
 */
ctr_object* ctr_build_string(char* stringValue, long size) {
	ctr_object* stringObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTSTRING);
	if (size != 0) {
		stringObject->value.svalue->value = ctr_heap_allocate( size*sizeof(char) );
		memcpy(stringObject->value.svalue->value, stringValue, ( sizeof(char) * size ) );
	}
	stringObject->value.svalue->vlen = size;
	stringObject->link = CtrStdString;
	return stringObject;
}

/**
 * @internal
 * BuildStringFromCString
 *
 * Creates a Citrine String from a 0 terminated C String.
 */
ctr_object* ctr_build_string_from_cstring(char* cstring) {
	return ctr_build_string(cstring, strlen(cstring));
}

/**
 * @internal
 * BuildEmptyString
 *
 * Creates an empty string object, use this to avoid using
 * the 'magic' number 0 when building a string, it is more
 * readable this way and your intention is clearer.
 */
ctr_object* ctr_build_empty_string() {
	return ctr_build_string( "", 0 );
}

/**
 * [String] bytes
 *
 * Returns the number of bytes in a string, as opposed to
 * length which returns the number of UTF-8 code points (symbols or characters).
 */
ctr_object* ctr_string_bytes(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_float((float)myself->value.svalue->vlen);
}

/**
 * [String] = [other]
 *
 * Returns True if the other string is the same (in bytes).
 */
ctr_object* ctr_string_eq(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* other = ctr_internal_cast2string( argumentList->object );
	if (other->value.svalue->vlen != myself->value.svalue->vlen) {
		return ctr_build_bool(0);
	}
	return ctr_build_bool((strncmp(other->value.svalue->value, myself->value.svalue->value, myself->value.svalue->vlen)==0));
}

/**
 * [String] != [other]
 *
 * Returns True if the other string is not the same (in bytes).
 */
ctr_object* ctr_string_neq(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* other = ctr_internal_cast2string( argumentList->object );
	if (other->value.svalue->vlen != myself->value.svalue->vlen) {
		return ctr_build_bool(1);
	}
	return ctr_build_bool(!(strncmp(other->value.svalue->value, myself->value.svalue->value, myself->value.svalue->vlen)==0));
}

/**
 * [String] length
 *
 * Returns the length of the string in symbols.
 * This message is UTF-8 unicode aware. A 4 byte character will be counted as ONE.
 */
ctr_object* ctr_string_length(ctr_object* myself, ctr_argument* argumentList) {
	ctr_size n = ctr_getutf8len(myself->value.svalue->value, (ctr_size) myself->value.svalue->vlen);
	return ctr_build_number_from_float((ctr_number) n);
}

/**
 * [String] + [other]
 *
 * Appends other string to self and returns the resulting
 * string as a new object.
 */
ctr_object* ctr_string_concat(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* strObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTSTRING);
	ctr_size n1;
	ctr_size n2;
	char* dest;
	ctr_object* newString;
	strObject = ctr_internal_cast2string(argumentList->object);
	n1 = myself->value.svalue->vlen;
	n2 = strObject->value.svalue->vlen;
	dest = ctr_heap_allocate( sizeof(char) * ( n1 + n2 ) );
	memcpy(dest, myself->value.svalue->value, n1);
	memcpy(dest+n1, strObject->value.svalue->value, n2);
	newString = ctr_build_string(dest, (n1 + n2));
	ctr_heap_free( dest );
	return newString;
}

/**
 * [String] append: [String].
 *
 * Appends the specified string to itself. This is different from the '+'
 * message, the '+' message adds the specified string while creating a new string.
 * Appends on the other hand modifies the original string.
 *
 * Usage:
 *
 * x := 'Hello '.
 * x append: 'World'.
 * Pen write: x. #Hello World
 *
 * Instead of using the append message you may also use its short form,
 * like this:
 *
 * x +=: 'World'.
 */
ctr_object* ctr_string_append(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* strObject;
	ctr_size n1;
	ctr_size n2;
	char* dest;
	strObject = ctr_internal_cast2string(argumentList->object);
	n1 = myself->value.svalue->vlen;
	n2 = strObject->value.svalue->vlen;
	if ( n1 < 0 || n2 < 0 ) {
		fprintf(stderr, "Invalid String length detected.\n");
		exit(1);
	}
	if ( ( n1 + n2 ) == 0 ) return myself;
	dest = ctr_heap_allocate( sizeof( char ) * ( n1 + n2 ) );
	memcpy(dest, myself->value.svalue->value, n1);
	memcpy(dest+n1, strObject->value.svalue->value, n2);
	if ( n1 > 0 ) {
		ctr_heap_free( myself->value.svalue->value );
	}
	myself->value.svalue->value = dest;
	myself->value.svalue->vlen  = (n1 + n2);
	return myself;
}

/**
 * [String] from: [position] to: [destination]
 *
 * Returns a portion of a string defined by from-to values.
 * This message is UTF-8 unicode aware.
 *
 * Usage:
 *
 * 'hello' from: 2 to: 3. #ll
 */
ctr_object* ctr_string_fromto(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* fromPos = ctr_internal_cast2number(argumentList->object);
	ctr_object* toPos = ctr_internal_cast2number(argumentList->next->object);
	long len = myself->value.svalue->vlen;
	long a = (fromPos->value.nvalue);
	long b = (toPos->value.nvalue);
	long t;
	long ua, ub;
	char* dest;
	ctr_object* newString;
	if (b == a) return ctr_build_empty_string();
	if (a > b) {
		t = a; a = b; b = t;
	}
	if (a > len) return ctr_build_empty_string();
	if (b > len) b = len;
	if (a < 0) a = 0;
	if (b < 0) return ctr_build_empty_string();
	ua = getBytesUtf8(myself->value.svalue->value, 0, a);
	ub = getBytesUtf8(myself->value.svalue->value, ua, ((b - a)));
	dest = ctr_heap_allocate( ub * sizeof(char) );
	memcpy(dest, (myself->value.svalue->value) + ua, ub);
	newString = ctr_build_string(dest,ub);
	ctr_heap_free( dest );
	return newString;
}

/**
 * [String] from: [start] length: [length]
 *
 * Returns a portion of a string defined by from
 * and length values.
 * This message is UTF-8 unicode aware.
 *
 * Usage:
 *
 * 'hello' from: 2 length: 3. #llo
 */
ctr_object* ctr_string_from_length(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* fromPos = ctr_internal_cast2number(argumentList->object);
	ctr_object* length = ctr_internal_cast2number(argumentList->next->object);
	long len = myself->value.svalue->vlen;
	long a = (fromPos->value.nvalue);
	long b = (length->value.nvalue);
	long ua, ub;
	char* dest;
	ctr_object* newString;
	if (b == 0) return ctr_build_empty_string();
	if (b < 0) {
		a = a + b;
		b = labs(b);
	}
	if (a < 0) a = 0;
	if (a > len) a = len;
	if ((a + b)>len) b = len - a;
	if ((a + b)<0) b = b - a;
	ua = getBytesUtf8(myself->value.svalue->value, 0, a);
	ub = getBytesUtf8(myself->value.svalue->value, ua, b);
	dest = ctr_heap_allocate( ub * sizeof(char) );
	memcpy(dest, (myself->value.svalue->value) + ua, ub);
	newString = ctr_build_string(dest,ub);
	ctr_heap_free( dest );
	return newString;
}

/**
 * [String] skip: [number]
 *
 * Returns a string without the first X characters.
 */
ctr_object* ctr_string_skip(ctr_object* myself, ctr_argument* argumentList) {
	ctr_argument* argument1;
	ctr_argument* argument2;
	ctr_object* result;
	ctr_size textLength;
	if (myself->value.svalue->vlen < argumentList->object->value.nvalue) return ctr_build_empty_string();
	argument1 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	argument2 = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	argument1->object = argumentList->object;
	argument1->next = argument2;
	textLength = ctr_getutf8len(myself->value.svalue->value, (ctr_size) myself->value.svalue->vlen);
	argument2->object = ctr_build_number_from_float(textLength - argumentList->object->value.nvalue);
	result = ctr_string_from_length(myself, argument1);
	ctr_heap_free( argument1 );
	ctr_heap_free( argument2 );
	return result;
}


/**
 * [String] at: [position]
 *
 * Returns the character at the specified position (UTF8 aware).
 * You may also use the alias '@'.
 *
 * Usage:
 *
 * ('hello' at: 2). #l
 * ('hello' @ 2). #l
 */
ctr_object* ctr_string_at(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* fromPos = ctr_internal_cast2number(argumentList->object);
	ctr_size a = (ctr_size) (fromPos->value.nvalue);
	ctr_size textLength = ctr_getutf8len(myself->value.svalue->value, (ctr_size) myself->value.svalue->vlen);
	if (a < 0) return CtrStdNil;
	if (a > textLength) return CtrStdNil;
	long ua = getBytesUtf8(myself->value.svalue->value, 0, a);
	long ub = getBytesUtf8(myself->value.svalue->value, ua, 1);
	ctr_object* newString;
	char* dest = ctr_heap_allocate( ub * sizeof( char ) );
	memcpy(dest, (myself->value.svalue->value) + ua, ub);
	newString = ctr_build_string(dest,ub);
	ctr_heap_free( dest );
	return newString;
}

/**
 * [String] byteAt: [position]
 *
 * Returns the byte at the specified position (in bytes).
 * Note that you cannot use the '@' message here because that will
 * return the unicode point at the specified position, not the byte.
 *
 * Usage:
 * ('abc' byteAt: 1). #98
 */
ctr_object* ctr_string_byte_at(ctr_object* myself, ctr_argument* argumentList) {
	char x;
	ctr_object* fromPos = ctr_internal_cast2number(argumentList->object);
	long a = (fromPos->value.nvalue);
	long len = myself->value.svalue->vlen;
	if (a > len) return CtrStdNil;
	if (a < 0) return CtrStdNil;
	x = (char) *(myself->value.svalue->value + a);
	return ctr_build_number_from_float((double)x);
}

/**
 * [String] indexOf: [subject]
 *
 * Returns the index (character number, not the byte!) of the
 * needle in the haystack.
 *
 * Usage:
 *
 * 'find the needle' indexOf: 'needle'. #9
 *
 */
ctr_object* ctr_string_index_of(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* sub = ctr_internal_cast2string(argumentList->object);
	long hlen = myself->value.svalue->vlen;
	long nlen = sub->value.svalue->vlen;
	uintptr_t byte_index;
	ctr_size uchar_index;
	char* p = ctr_internal_memmem(myself->value.svalue->value, hlen, sub->value.svalue->value, nlen, 0);
	if (p == NULL) return ctr_build_number_from_float((ctr_number)-1);
	byte_index = (uintptr_t) p - (uintptr_t) (myself->value.svalue->value);
	uchar_index = ctr_getutf8len(myself->value.svalue->value, byte_index);
	return ctr_build_number_from_float((ctr_number) uchar_index);
}

/**
 * [String] asciiUpperCase
 *
 * Returns a new uppercased version of the string.
 * Note that this is just basic ASCII case functionality, this should only
 * be used for internal keys and as a basic utility function. This function
 * DOES NOT WORK WITH UTF8 characters !
 */
ctr_object* ctr_string_to_upper(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* newString = NULL;
	char* str = myself->value.svalue->value;
	size_t  len = myself->value.svalue->vlen;
	char* tstr = ctr_heap_allocate( len * sizeof( char ) );
	int i=0;
	for(i =0; i < len; i++) {
		tstr[i] = toupper(str[i]);
	}
	newString = ctr_build_string(tstr, len);
	ctr_heap_free( tstr );
	return newString;
}


/**
 * [String] asciiLowerCase
 *
 * Returns a new lowercased version of the string.
 * Note that this is just basic ASCII case functionality, this should only
 * be used for internal keys and as a basic utility function. This function
 * DOES NOT WORK WITH UTF8 characters !
 */
ctr_object* ctr_string_to_lower(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* newString = NULL;
	char* str = myself->value.svalue->value;
	size_t len = myself->value.svalue->vlen;
	char* tstr = ctr_heap_allocate( len * sizeof( char ) );
	int i=0;
	for(i =0; i < len; i++) {
		tstr[i] = tolower(str[i]);
	}
	newString = ctr_build_string(tstr, len);
	ctr_heap_free( tstr );
	return newString;
}

/**
 * [String] asciiLowerCase1st
 *
 * Converts the first character of the recipient to lowercase and
 * returns the resulting string object.
 */
ctr_object* ctr_string_to_lower1st(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* newString = NULL;
	size_t len = myself->value.svalue->vlen;
	if (len == 0) return ctr_build_empty_string();
	char* tstr = ctr_heap_allocate( len * sizeof( char ) );
	strncpy(tstr, myself->value.svalue->value, len);
	tstr[0] = tolower(tstr[0]);
	newString = ctr_build_string(tstr, len);
	ctr_heap_free( tstr );
	return newString;
}

/**
 * [String] asciiUpperCase1st
 *
 * Converts the first character of the recipient to uppercase and
 * returns the resulting string object.
 */
ctr_object* ctr_string_to_upper1st(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* newString;
	size_t len = myself->value.svalue->vlen;
	if (len == 0) return ctr_build_empty_string();
	char* tstr = ctr_heap_allocate( len * sizeof( char ) );
	strncpy(tstr, myself->value.svalue->value, len);
	tstr[0] = toupper(tstr[0]);
	newString = ctr_build_string(tstr, len);
	ctr_heap_free( tstr );
	return newString;
}

ctr_object* ctr_string_to_string(ctr_object* myself, ctr_argument* argumentList) {
	return myself;
}

/**
 * [String] lastIndexOf: [subject]
 *
 * Returns the index (character number, not the byte!) of the
 * needle in the haystack.
 *
 * Usage:
 *
 * 'find the needle' lastIndexOf: 'needle'. #9
 */
ctr_object* ctr_string_last_index_of(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* sub = ctr_internal_cast2string(argumentList->object);
	ctr_size hlen = myself->value.svalue->vlen;
	ctr_size nlen = sub->value.svalue->vlen;
	ctr_size uchar_index;
	ctr_size byte_index;
	char* p = ctr_internal_memmem( myself->value.svalue->value, hlen, sub->value.svalue->value, nlen, 1 );
	if (p == NULL) return ctr_build_number_from_float((float)-1);
	byte_index = (ctr_size) ( p - (myself->value.svalue->value) );
	uchar_index = ctr_getutf8len(myself->value.svalue->value, byte_index);
	return ctr_build_number_from_float((float) uchar_index);
}

/**
 * [String] replace: [string] with: [other]
 *
 * Replaces needle with replacement in original string and returns
 * the result as a new string object.
 *
 * Usage:
 *
 * 'LiLo BootLoader' replace: 'L' with: 'l'. #lilo Bootloader
 */
ctr_object* ctr_string_replace_with(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* needle = ctr_internal_cast2string(argumentList->object);
	ctr_object* replacement = ctr_internal_cast2string(argumentList->next->object);
	ctr_object* str;
	char* dest;
	char* odest;
	char* src = myself->value.svalue->value;
	char* ndl = needle->value.svalue->value;
	char* rpl = replacement->value.svalue->value;
	long hlen = myself->value.svalue->vlen;
	long nlen = needle->value.svalue->vlen;
	long rlen = replacement->value.svalue->vlen;
	long dlen = hlen;
	char* p;
	long i = 0;
	long offset = 0;
	long d;
	if (nlen == 0 || hlen == 0) {
		return ctr_build_string(src, hlen);
	}
	dest = (char*) ctr_heap_allocate( dlen * sizeof( char ) );
	odest = dest;
	while(1) {
		p = ctr_internal_memmem(src, hlen, ndl, nlen, 0);
		if (p == NULL) break;
		d = (dest - odest);
		if ((dlen - nlen + rlen)>dlen) {
			dlen = (dlen - nlen + rlen);
			odest = (char*) ctr_heap_reallocate(odest, dlen * sizeof(char) );
			dest = (odest + d);
		} else {
			dlen = (dlen - nlen + rlen);
		}
		offset = (p - src);
		memcpy(dest, src, offset);
		dest = dest + offset;
		memcpy(dest, rpl, rlen);
		dest = dest + rlen;
		hlen = hlen - (offset + nlen);
		src  = src + (offset + nlen);
		i++;
	}
	memcpy(dest, src, hlen);
	str = ctr_build_string(odest, dlen);
	ctr_heap_free( odest );
	return str;
}

/**
 * [String] findPattern: [String] do: [Block] options: [String].
 *
 * Matches the POSIX regular expression in the first argument against
 * the string and executes the specified block on every match passing
 * an array containing the matches.
 *
 * The options parameter can be used to pass specific flags to the
 * regular expression engine. As of the moment of writing this functionality
 * has not been implemented yet. The only flag you can set at this moment is
 * the 'ignore' flag, just a test flag. This flag does not execute the block.
 *
 * Usage:
 *
 * 'hello world' findPattern: '([hl])' do: { :arr
 *  Pen write: (arr join: '|'), brk.
 * } options: ''.
 *
 * On every match the block gets executed and the matches are
 * passed to the block as arguments. You can also use this feature to replace
 * parts of the string, simply return the replacement string in your block.
 */
ctr_object* ctr_string_find_pattern_options_do( ctr_object* myself, ctr_argument* argumentList ) {
	regex_t pattern;
	int reti;
	int sticky1, sticky2, sticky3;
	int regex_error = 0;
	size_t n = 255;
	size_t i = 0;
	regmatch_t matches[255];
	char* needle = ctr_heap_allocate_cstring( argumentList->object );
	char* options = ctr_heap_allocate_cstring( argumentList->next->next->object );
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
	int eflags = REG_EXTENDED;
	if (flagNewLine) eflags |= REG_NEWLINE;
	if (flagCI) eflags |= REG_ICASE;
	reti = regcomp(&pattern, needle, eflags);
	if ( reti ) {
		CtrStdFlow = ctr_build_string_from_cstring( "Could not compile regular expression." );
		CtrStdFlow->info.sticky=1;
		return CtrStdNil;
	}
	char* haystack = ctr_heap_allocate_cstring(myself);
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
		block->info.sticky = 1;
		blockArguments->object->info.sticky = 1;
		newString->info.sticky = 1;
		ctr_object* replacement = replacement = ctr_block_run( block, blockArguments, block );
		block->info.sticky = sticky1;
		blockArguments->object->info.sticky = sticky2;
		newString->info.sticky = sticky3;
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

/**
 * [String] findPattern: [String] do: [Block].
 *
 * Same as findPattern:do:options: but without the options, no flags will
 * be send to the regex engine.
 */
ctr_object* ctr_string_find_pattern_do( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_argument* no_options = ctr_heap_allocate( sizeof( ctr_argument ) );
	argumentList->next->next->object = ctr_build_empty_string();
	ctr_object* answer;
	answer = ctr_string_find_pattern_options_do( myself, argumentList );
	ctr_heap_free( no_options );
	return answer;
}

/**
 * [String] contains: [String]
 *
 * Returns True if the other string is a substring.
 */
ctr_object* ctr_string_contains( ctr_object* myself, ctr_argument* argumentList ) {
	return ctr_build_bool(
		ctr_internal_cast2number(
			ctr_string_index_of( myself, argumentList )
		)->value.nvalue > -1
	);
}

/**
 * [String] containsPattern: [String].
 *
 * Tests the pattern against the string and returns True if there is a match
 * and False otherwise.
 *
 * Usage:
 *
 * var match := 'Hello World' containsPattern: '[:space:]'.
 * #match will be True because there is a space in 'Hello World'
 */
ctr_object* ctr_string_contains_pattern( ctr_object* myself, ctr_argument* argumentList ) {
	regex_t pattern;
	int regex_error = 0;
	int result = 0;
	char* error_message = ctr_heap_allocate( 255 );
	char* needle = ctr_heap_allocate_cstring( argumentList->object );
	char* haystack = ctr_heap_allocate_cstring(myself);
	ctr_object* answer;
	regex_error = regcomp(&pattern, needle, REG_EXTENDED);
	if ( regex_error ) {
		CtrStdFlow = ctr_error_text( "Could not compile regular expression." );
		answer = CtrStdNil;
	} else {
		result = regexec(&pattern, haystack, 0, NULL, 0 );
		if ( !result ) {
			answer = ctr_build_bool( 1 );
		} else if ( result == REG_NOMATCH ) {
			answer = ctr_build_bool( 0 );
		} else {
			CtrStdFlow = ctr_build_string_from_cstring( error_message );
			answer = CtrStdNil;
		}
	}
	regfree( &pattern );
	ctr_heap_free( error_message );
	ctr_heap_free( needle );
	ctr_heap_free( haystack );
	return answer;
}

/**
 * [String] trim
 *
 * Trims a string. Removes surrounding white space characters
 * from string and returns the result as a new string object.
 *
 * Usage:
 *
 * ' hello ' trim. #hello
 *
 * The example above will strip all white space characters from the
 * recipient on both sides of the text. Also see: leftTrim and rightTrim
 * for variations of this message.
 */
ctr_object* ctr_string_trim(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* newString = NULL;
	char* str = myself->value.svalue->value;
	long  len = myself->value.svalue->vlen;
	long i, begin, end, tlen;
	char* tstr;
	if (len == 0) return ctr_build_empty_string();
	i = 0;
	while(i < len && isspace(*(str+i))) i++;
	begin = i;
	i = len - 1;
	while(i > begin && isspace(*(str+i))) i--;
	end = i + 1;
	tlen = (end - begin);
	tstr = ctr_heap_allocate( tlen * sizeof( char ) );
	memcpy(tstr, str+begin, tlen);
	newString = ctr_build_string(tstr, tlen);
	ctr_heap_free( tstr );
	return newString;
}


/**
 * [String] leftTrim
 *
 * Removes all the whitespace at the left side of the string.
 *
 * Usage:
 *
 * message := ' hello world  '.
 * message leftTrim.
 *
 * The example above will remove all the whitespace at the left of the
 * string but leave the spaces at the right side intact.
 */
ctr_object* ctr_string_ltrim(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* newString = NULL;
	char* str = myself->value.svalue->value;
	long  len = myself->value.svalue->vlen;
	long i = 0, begin;
	long tlen;
	char* tstr;
	if (len == 0) return ctr_build_empty_string();
	while(i < len && isspace(*(str+i))) i++;
	begin = i;
	i = len - 1;
	tlen = (len - begin);
	tstr = ctr_heap_allocate( tlen * sizeof(char) );
	memcpy(tstr, str+begin, tlen);
	newString = ctr_build_string(tstr, tlen);
	ctr_heap_free( tstr );
	return newString;
}


ctr_object* ctr_string_padding(ctr_object* myself, ctr_argument* argumentList, int left) {
	uint16_t padding;
	char* buffer;
	char* format;
	char* stringParam;
	ctr_size bufferSize;
	ctr_argument* a;
	ctr_object* answer;
	ctr_object* formatObj;
	if (left == 1) {
		formatObj = ctr_build_string_from_cstring( "%" );
	} else {
		formatObj = ctr_build_string_from_cstring( "%-" );
	}
	a = ctr_heap_allocate( sizeof(ctr_argument) );
	padding = (uint16_t) ctr_internal_cast2number( argumentList->object )->value.nvalue;
	a->object = ctr_internal_cast2string( ctr_build_number_from_float( (ctr_number) padding ) );
	formatObj = ctr_string_concat( formatObj, a );
	a->object = ctr_build_string_from_cstring( "s" );
	formatObj = ctr_string_concat( formatObj, a );
	format = ctr_heap_allocate_cstring(formatObj);
	bufferSize = ( myself->value.svalue->vlen + padding + 1);
	buffer = ctr_heap_allocate( bufferSize );
	stringParam = ctr_heap_allocate_cstring( myself );
	snprintf(buffer, bufferSize, format, stringParam );
	answer = ctr_build_string_from_cstring( buffer );
	ctr_heap_free(buffer);
	ctr_heap_free(stringParam);
	ctr_heap_free(format);
	ctr_heap_free(a);
	return answer;
}

/**
 * [String] paddingLeft: [Number].
 *
 * Adds the specified number of spaces to the left of the string.
 */
ctr_object* ctr_string_padding_left(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_string_padding( myself, argumentList, 1);
}

/**
 * [String] paddingLeft: [Number].
 *
 * Adds the specified number of spaces to the right of the string.
 */
ctr_object* ctr_string_padding_right(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_string_padding( myself, argumentList, 0);
}

/**
 * [String] rightTrim
 *
 * Removes all the whitespace at the right side of the string.
 *
 * Usage:
 *
 * message := ' hello world  '.
 * message rightTrim.
 *
 * The example above will remove all the whitespace at the right of the
 * string but leave the spaces at the left side intact.
 */
ctr_object* ctr_string_rtrim(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* newString = NULL;
	char* str = myself->value.svalue->value;
	long  len = myself->value.svalue->vlen;
	long i = 0, end, tlen;
	char* tstr;
	if (len == 0) return ctr_build_empty_string();
	i = len - 1;
	while(i > 0 && isspace(*(str+i))) i--;
	end = i + 1;
	tlen = end;
	tstr = ctr_heap_allocate( tlen * sizeof(char) );
	memcpy(tstr, str, tlen);
	newString = ctr_build_string(tstr, tlen);
	ctr_heap_free( tstr );
	return newString;
}

/**
 * [String] toNumber
 *
 * Converts string to a number.
 */
ctr_object* ctr_string_to_number(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_number_from_string(myself->value.svalue->value, myself->value.svalue->vlen);
}

/**
 * [String] toBoolean
 *
 * Converts string to boolean
 */
ctr_object* ctr_string_to_boolean(ctr_object* myself, ctr_argument* argumentList) {
	if ( myself->value.svalue->vlen == 0 ) return ctr_build_bool(0);
	return ctr_build_bool( 1 );
}

/**
 * [String] split: [String]
 *
 * Converts a string to an array by splitting the string using
 * the specified delimiter (also a string).
 */
ctr_object* ctr_string_split(ctr_object* myself, ctr_argument* argumentList) {
	char* str = myself->value.svalue->value;
	long len = myself->value.svalue->vlen;
	ctr_object* delimObject  = ctr_internal_cast2string(argumentList->object);
	char* dstr = delimObject->value.svalue->value;
	long dlen = delimObject->value.svalue->vlen;
	ctr_argument* arg;
	char* elem;
	ctr_object* arr = ctr_array_new(CtrStdArray, NULL);
	long i;
	long j = 0;
	char* buffer = ctr_heap_allocate( sizeof(char)*len );
	for(i=0; i<len; i++) {
		buffer[j] = str[i];
		j++;
		if (ctr_internal_memmem(buffer, j, dstr, dlen, 0)!=NULL) {
			elem = ctr_heap_allocate( sizeof( char ) * ( j - dlen ) );
			memcpy(elem,buffer,j-dlen);
			arg = ctr_heap_allocate( sizeof( ctr_argument ) );
			arg->object = ctr_build_string(elem, j-dlen);
			ctr_array_push(arr, arg);
			ctr_heap_free( arg );
			ctr_heap_free( elem );
			j=0;
		}
	}
	if (j>0) {
		elem = ctr_heap_allocate( sizeof( char ) * j );
		memcpy(elem,buffer,j);
		arg = ctr_heap_allocate( sizeof( ctr_argument ) );
		arg->object = ctr_build_string(elem, j);
		ctr_array_push(arr, arg);
		ctr_heap_free( arg );
		ctr_heap_free( elem );
	}
	ctr_heap_free( buffer );
	return arr;
}

/**
 * [String] characters.
 *
 * Splits the string in UTF-8 characters and returns
 * those as an array.
 *
 * Usage:
 *
 * a := 'abc' characters.
 * a count. #3
 */
ctr_object* ctr_string_characters( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_size i;
	int charSize;
	ctr_object* arr;
	ctr_argument* newArgumentList;
	arr = ctr_array_new(CtrStdArray, NULL);
	newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	i = 0;
	while( i < myself->value.svalue->vlen ) {
		charSize = ctr_utf8size( *(myself->value.svalue->value + i) );
		newArgumentList->object = ctr_build_string( myself->value.svalue->value + i, charSize );
		ctr_array_push( arr, newArgumentList );
		i += charSize;
	}
	ctr_heap_free( newArgumentList );
	return arr;
}

/**
 * [String] toByteArray
 *
 * Returns an array of bytes representing the string.
 */
ctr_object* ctr_string_to_byte_array( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_size i;
	ctr_object* arr;
	ctr_argument* newArgumentList;
	arr = ctr_array_new(CtrStdArray, NULL);
	newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	i = 0;
	while( i < myself->value.svalue->vlen ) {
		newArgumentList->object = ctr_build_number_from_float( (double) (uint8_t) *(myself->value.svalue->value + i) );
		ctr_array_push( arr, newArgumentList );
		i ++;
	}
	ctr_heap_free( newArgumentList );
	return arr;
}

/**
 * [String] appendByte: [Number].
 *
 * Appends a raw byte to a string.
 */
ctr_object* ctr_string_append_byte( ctr_object* myself, ctr_argument* argumentList ) {
	char* dest;
	char byte;
	byte = (uint8_t) ctr_internal_cast2number( argumentList->object )->value.nvalue;
	dest = ctr_heap_allocate( myself->value.svalue->vlen + 1 );
	memcpy( dest, myself->value.svalue->value, myself->value.svalue->vlen );
	*( dest + myself->value.svalue->vlen ) = byte;
	if ( myself->value.svalue->vlen > 0 ) {
		ctr_heap_free( myself->value.svalue->value );
	}
	myself->value.svalue->value = dest;
	myself->value.svalue->vlen++;
	return myself;
}

/**
 * [String] compare: [String]
 *
 * Compares a string using the UTF-8 compatible strcmp function.
 *
 * Usage:
 *
 * word compare: other.
 */
ctr_object* ctr_string_compare( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_size maxlen;
	if (myself->value.svalue->vlen < argumentList->object->value.svalue->vlen) {
		maxlen = myself->value.svalue->vlen;
	} else {
		maxlen = argumentList->object->value.svalue->vlen;
	}
	return ctr_build_number_from_float( (ctr_number) strncmp(
		myself->value.svalue->value,
		ctr_internal_cast2string(argumentList->object)->value.svalue->value,
		maxlen
	) );
}

/**
 * [String] < [String]
 *
 * Returns True if the first String comes before the latter
 * alphabetically. The actual comparison is based on the UTF-8 compatible
 * function strcmp.
 */
ctr_object* ctr_string_before(ctr_object* myself, ctr_argument* argumentList ) {
	if ( ctr_string_compare( myself, argumentList )->value.nvalue < 0 ) {
		return ctr_build_bool( 1 );
	}
	return ctr_build_bool( 0 );
}

/**
 * [String] ≤ [String]
 *
 * Returns True if the first String comes before or at the same
 * position as the latter alphabetically. The actual comparison is based on the UTF-8 compatible
 * function strcmp.
 */
ctr_object* ctr_string_before_or_same(ctr_object* myself, ctr_argument* argumentList ) {
	if ( ctr_string_compare( myself, argumentList )->value.nvalue <= 0 ) {
		return ctr_build_bool( 1 );
	}
	return ctr_build_bool( 0 );
}

/**
 * [String] > [String]
 *
 * Returns True if the first String comes after the latter
 * alphabetically. The actual comparison is based on the UTF-8 compatible
 * function strcmp.
 */
ctr_object* ctr_string_after(ctr_object* myself, ctr_argument* argumentList ) {
	if ( ctr_string_compare( myself, argumentList )->value.nvalue > 0 ) {
		return ctr_build_bool( 1 );
	}
	return ctr_build_bool( 0 );
}

/**
 * [String] ≥ [String]
 *
 * Returns True if the first String comes after or at the same position as the latter
 * alphabetically. The actual comparison is based on the UTF-8 compatible
 * function strcmp.
 */
ctr_object* ctr_string_after_or_same(ctr_object* myself, ctr_argument* argumentList ) {
	if ( ctr_string_compare( myself, argumentList )->value.nvalue >= 0 ) {
		return ctr_build_bool( 1 );
	}
	return ctr_build_bool( 0 );
}

/**
 * [String] escape: '\n'.
 *
 * Escapes the specified ASCII character in a string.
 * If the character is a control character, the well known
 * C-based character substitute will be used.
 */
ctr_object* ctr_string_escape(ctr_object* myself, ctr_argument* argumentList)  {
	ctr_object* escape = argumentList->object;
	ctr_object* newString = NULL;
	char* str = myself->value.svalue->value;
	long  len = myself->value.svalue->vlen;
	char* tstr;
	long i=0;
	long k=0;
	ctr_size q = 0;
	ctr_size numOfCharacters = 0;
	long tlen = 0;
	char* characters;
	char character;
	char characterDescription;
	char isControlChar = 0;
	char escaped;
	long tag_len = 0;
	characters = escape->value.svalue->value;
	numOfCharacters = escape->value.svalue->vlen;
	if (numOfCharacters < 1) {
		return myself;
	}
	for (q = 0; q < numOfCharacters; q ++) {
		character = characters[q];
		isControlChar = 0;
		characterDescription = character;
		for(i =0; i < len; i++) {
			char c = str[i];
			if (c == character) {
				tag_len += 2;
			}
		}
	}
	tlen = len + tag_len;
	tstr = ctr_heap_allocate( tlen * sizeof( char ) );
	for(i = 0; i < len; i++) {
		char c = str[i];
		escaped = 0;
		for (q = 0; q < numOfCharacters; q ++) {
			character = characters[q];
			isControlChar = 0;
			if (character == '\t') {
				characterDescription = 't';
				isControlChar = 1;
			}
			if (character == '\r') {
				characterDescription = 'r';
				isControlChar = 1;
			}
			if (character == '\n') {
				characterDescription = 'n';
				isControlChar = 1;
			}
			if (character == '\b') {
				characterDescription = 'b';
				isControlChar = 1;
			}
			if (c == character) {
				tstr[k++] = '\\';
				if (isControlChar) {
					tstr[k++] = characterDescription;
				} else {
					tstr[k++] = str[i];
				}
				escaped = 1;
				break;
			}
		}
		if (!escaped) {
			tstr[k++] = str[i];
		}
	}
	newString = ctr_build_string(tstr, tlen);
	ctr_heap_free( tstr );
	return newString;
}

/**
 * [String] unescape: '\n'.
 *
 * 'UnEscapes' the specified ASCII character in a string.
 */
ctr_object* ctr_string_unescape(ctr_object* myself, ctr_argument* argumentList)  {
	ctr_object* escape = argumentList->object;
	ctr_object* newString = NULL;
	char character;
	char characterDescription;
	char* str = myself->value.svalue->value;
	long  len = myself->value.svalue->vlen;
	char* tstr;
	char isControlChar = 0;
	char* characters;
	ctr_size numOfCharacters;
	char unescaped;
	ctr_size q;
	long i=0;
	long k=0;
	long tlen = 0;
	long tag_len = 0;
	characters = escape->value.svalue->value;
	numOfCharacters = escape->value.svalue->vlen;
	if (numOfCharacters < 1) {
		return myself;
	}
	for (q = 0; q < numOfCharacters; q ++) {
		character = characters[q];
		isControlChar = 0;
		characterDescription = character;
		if (character == '\t') {
			characterDescription = 't';
			isControlChar = 1;
		}
		if (character == '\r') {
			characterDescription = 'r';
			isControlChar = 1;
		}
		if (character == '\n') {
			characterDescription = 'n';
			isControlChar = 1;
		}
		if (character == '\b') {
			characterDescription = 'b';
			isControlChar = 1;
		}
		for(i = 0; i < len; i++) {
			if (i<len-1 && str[i] == '\\' && str[i+1] == characterDescription) {
				tag_len -= 1;
			}
		}
	}
	tlen = len + tag_len;
	tstr = ctr_heap_allocate( tlen * sizeof( char ) );
	for(i = 0; i < len; i++) {
		unescaped = 0;
		for (q = 0; q < numOfCharacters; q ++) {
			character = characters[q];
			characterDescription = character;
			isControlChar = 0;
			if (character == '\t') {
				characterDescription = 't';
				isControlChar = 1;
			}
			if (character == '\r') {
				characterDescription = 'r';
				isControlChar = 1;
			}
			if (character == '\n') {
				characterDescription = 'n';
				isControlChar = 1;
			}
			if (character == '\b') {
				characterDescription = 'b';
				isControlChar = 1;
			}
			if (i<len-1 && str[i] == '\\' && str[i+1] == characterDescription) {
				if (isControlChar) {
					if ( characterDescription == 'n' ) {
						tstr[k++] = '\n';
					}
					if ( characterDescription == 'r' ) {
						tstr[k++] = '\r';
					}
					if ( characterDescription == 't' ) {
						tstr[k++] = '\t';
					}
					if ( characterDescription == 'b' ) {
						tstr[k++] = '\b';
					}
				} else {
					tstr[k++] = str[i+1];
				}
				i++;
				unescaped = 1;
			}
		}
		if (unescaped == 0) {
			tstr[k++] = str[i];
		}
	}
	newString = ctr_build_string(tstr, tlen);
	ctr_heap_free( tstr );
	return newString;
}

/**
 * [String] htmlEscape
 *
 * Escapes HTML chars.
 */
ctr_object* ctr_string_html_escape(ctr_object* myself, ctr_argument* argumentList)  {
	ctr_object* newString = NULL;
	char* str = myself->value.svalue->value;
	long  len = myself->value.svalue->vlen;
	char* tstr;
	long i=0;
	long j=0;
	long k=0;
	long rlen;
	long tlen = 0;
	long tag_len = 0;
	long tag_rlen = 0;
	char* replacement;
	for(i =0; i < len; i++) {
		char c = str[i];
		switch(c) {
			case '<':
				tag_len += 4;
				tag_rlen += 1;
				break;
			case '>':
				tag_len += 4;
				tag_rlen += 1;
				break;
			case '&':
				tag_len += 5;
				tag_rlen += 1;
				break;
			case '"':
				tag_len += 6;
				tag_rlen += 1;
				break;
			case '\'':
				tag_len += 6;
				tag_rlen += 1;
				break;
			default:
				break;
		}
	}
	tlen = len + tag_len - tag_rlen;
	tstr = ctr_heap_allocate( tlen * sizeof( char ) );
	for(i = 0; i < len; i++) {
		char c = str[i];
		switch (c) {
			case '<':
				replacement = "&lt;";
				rlen = 4;
				for(j=0; j<rlen; j++) tstr[k++]=replacement[j];
				break;
			case '>':
				replacement = "&gt;";
				rlen = 4;
				for(j=0; j<rlen; j++) tstr[k++]=replacement[j];
				break;
			case '&':
				replacement = "&amp;";
				rlen = 5;
				for(j=0; j<rlen; j++) tstr[k++]=replacement[j];
				break;
			case '"':
				replacement = "&quot;";
				rlen = 6;
				for(j=0; j<rlen; j++) tstr[k++]=replacement[j];
				break;
			case '\'':
				replacement = "&apos;";
				rlen = 6;
				for(j=0; j<rlen; j++) tstr[k++]=replacement[j];
				break;
			default:
				tstr[k++] = str[i];
				break;
		}
	}
	newString = ctr_build_string(tstr, tlen);
	ctr_heap_free( tstr );
	return newString;
}

/**
 * [String] hashWithKey: [String]
 *
 * Returns the hash of the recipient String using the specified key.
 * The default hash in Citrine is the SipHash which is also used internally.
 * SipHash can protect against hash flooding attacks.
 */
ctr_object* ctr_string_hash_with_key( ctr_object* myself, ctr_argument* argumentList ) {
	char* keyString = ctr_heap_allocate_cstring( argumentList->object );
	if ( strlen( keyString ) < 16 ) {
		ctr_heap_free( keyString );
		CtrStdFlow = ctr_error_text( "Key must be exactly 16 bytes long." );
		return CtrStdNil;
	}
	uint64_t t = siphash24( myself->value.svalue->value, myself->value.svalue->vlen, keyString);
	char* dest = ctr_heap_allocate( 40 );
	snprintf( dest, 40, "%" PRIu64, t );
	ctr_object* hash = ctr_build_string_from_cstring( dest );
	ctr_heap_free( dest );
	ctr_heap_free( keyString );
	return hash;
}

/**
 * [String] eval
 *
 * Evaluates the contents of the string as code.
 * In contrast to other languages, an eval statement can only
 * execute a very limited set of messages. Typically only Array and
 * Map building can be performed using eval. Using eval in Citrine can
 * therefore be considered 'safe'.
 *
 * Usage:
 *
 * a := 'Array < 1 ; 2 ; 3' eval.
 * x := a @ 2. #3
 */
ctr_object* ctr_string_eval(ctr_object* myself, ctr_argument* argumentList) {
	ctr_tnode* parsedCode;
	char* pathString;
	ctr_object* result;
	ctr_object* code;
	/* activate white-list based security profile */
	ctr_program_security_profile ^= CTR_SECPRO_EVAL;
	pathString = ctr_heap_allocate_tracked(sizeof(char)*5);
	memcpy(pathString, "eval", 4);
	memcpy(pathString+4,"\0",1);
	/* add a return statement so we can catch result */
	ctr_argument* newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	newArgumentList->object = myself;
	code = ctr_string_append( ctr_build_string_from_cstring( "^ " ), newArgumentList );
	newArgumentList->object = ctr_build_string_from_cstring( "." );
	code = ctr_string_append( code, newArgumentList );
	ctr_program_length = code->value.svalue->vlen;
	parsedCode = ctr_cparse_parse(code->value.svalue->value, pathString);
	ctr_cwlk_subprogram++;
	result = ctr_cwlk_run(parsedCode);
	ctr_cwlk_subprogram--;
	if ( result == NULL ) result = CtrStdNil;
	ctr_heap_free( newArgumentList );
	ctr_program_security_profile ^= CTR_SECPRO_EVAL;
	return result;
}

/**
 * [String] escapeQuotes.
 *
 * Escapes all single quotes in a string. Sending this message to a
 * string will cause all single quotes (') to be replaced with (\').
 */
ctr_object* ctr_string_quotes_escape(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* answer;
	char* str;
	ctr_size len;
	ctr_size i;
	ctr_size j;
	len = myself->value.svalue->vlen;
	for( i = 0; i < myself->value.svalue->vlen; i++ ) {
		if ( *(myself->value.svalue->value + i) == '\'' ) {
			len++;
		}
	}
	str = ctr_heap_allocate( len + 1 );
	j = 0;
	for( i = 0; i < myself->value.svalue->vlen; i++ ) {
		if ( *(myself->value.svalue->value + i) == '\'' ) {
			str[j+i] = '\\';
			j++;
		}
		str[j+i] = *(myself->value.svalue->value + i);
	}
	answer = ctr_build_string_from_cstring( str );
	ctr_heap_free( str );
	return answer;
}

/**
 * Block
 *
 * Literal:
 *
 * { parameters (if any) here... code here... }
 *
 * each parameter has to be prefixed with
 * a colon (:).
 *
 * Examples:
 *
 * { Pen write: 'a simple code block'. } run.
 * { :param Pen write: param. } applyTo: 'write this!'.
 * { :a :b ^ a + b. } applyTo: 1 and: 2.
 * { :a :b :c ^ a + b + c. } applyTo: 1 and: 2 and: 3.
 *
 */
ctr_object* ctr_build_block(ctr_tnode* node) {
	ctr_object* codeBlockObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTBLOCK);
	codeBlockObject->value.block = node;
	codeBlockObject->link = CtrStdBlock;
	return codeBlockObject;
}

/**
 * [Block] applyTo: [object]
 *
 * Runs a block of code using the specified object as a parameter.
 * If you run a block using the messages 'run' or 'applyTo:', me/my will
 * refer to the block itself instead of the containing object.
 */
ctr_object* ctr_block_run(ctr_object* myself, ctr_argument* argList, ctr_object* my) {
	ctr_object* result;
	ctr_tnode* node = myself->value.block;
	ctr_tlistitem* codeBlockParts = node->nodes;
	ctr_tnode* codeBlockPart1 = codeBlockParts->node;
	ctr_tnode* codeBlockPart2 = codeBlockParts->next->node;
	ctr_tlistitem* parameterList = codeBlockPart1->nodes;
	ctr_tnode* parameter;
	ctr_object* a;
	int sticky;
	ctr_open_context();
	if (parameterList && parameterList->node) {
		parameter = parameterList->node;
		while(1) {
			if (parameter && argList->object) {
				a = argList->object;
				ctr_assign_value_to_local(ctr_build_string(parameter->value, parameter->vlen), a);
			}
			if (!argList->next) break;
			argList = argList->next;
			if (!parameterList->next) break;
			parameterList = parameterList->next;
			parameter = parameterList->node;
		}
	}
	if (my) {
		ctr_assign_value_to_local_by_ref(ctr_build_string_from_cstring( ctr_clex_keyword_me ), my ); /* me should always point to object, otherwise you have to store me in self and cant use in if */
		ctr_assign_value_to_local_by_ref(ctr_build_string_from_cstring( ctr_clex_keyword_me_icon ), my );
	}
	ctr_assign_value_to_local(ctr_build_string_from_cstring( "thisBlock" ), myself ); /* otherwise running block may get gc'ed. */
	result = ctr_cwlk_run(codeBlockPart2);
	if (result == NULL) {
		if (my) result = my; else result = myself;
	}
	ctr_close_context();
	/* assign result to lower context to prevent it from being GC'ed. */
	ctr_internal_object_set_property( ctr_contexts[ctr_context_id], ctr_build_string_from_cstring(".rs"), result, CTR_CATEGORY_PRIVATE_PROPERTY );
	if (CtrStdFlow != NULL && CtrStdFlow != CtrStdBreak && CtrStdFlow != CtrStdContinue) {
		ctr_object* catchBlock = ctr_internal_create_object( CTR_OBJECT_TYPE_OTBLOCK );
		catchBlock = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "catch" ), 0);
		if (catchBlock != NULL) {
			ctr_argument* a = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
			a->object = ctr_internal_cast2string(CtrStdFlow);
			CtrStdFlow = NULL;
			sticky = a->object->info.sticky;
			a->object->info.sticky = 1;
			ctr_block_run(catchBlock, a, my);
			a->object->info.sticky = sticky;
			ctr_heap_free( a );
			result = myself;
		}
	}
	return result;
}

/**
 * [Block] whileTrue: [block]
 *
 * Runs a block of code, depending on the outcome runs the other block
 * as long as the result of the first one equals boolean True.
 *
 * Usage:
 *
 * x := 0.
 * { ^(x < 6). } whileFalse:
 * { x add: 1. }. #increment x until it reaches 6.
 *
 * Here we increment variable x by one until it reaches 6.
 * While the number x is lower than 6 we keep incrementing it.
 * Don't forget to use the return ^ symbol in the first block.
 */
ctr_object* ctr_block_while_true(ctr_object* myself, ctr_argument* argumentList) {
	int sticky1, sticky2;
	sticky1 = myself->info.sticky;
	sticky2 = argumentList->object->info.sticky;
	myself->info.sticky = 1;
	argumentList->object->info.sticky = 1;
	while (1 && !CtrStdFlow) {
		ctr_object* result = ctr_internal_cast2bool(ctr_block_run(myself, argumentList, NULL));
		if (result->value.bvalue == 0 || CtrStdFlow) break;
		ctr_block_run(argumentList->object, argumentList, NULL);
		if (CtrStdFlow == CtrStdContinue) CtrStdFlow = NULL; /* consume continue */
	}
	if (CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL; /* consume break */
	myself->info.sticky = sticky1;
	argumentList->object->info.sticky = sticky2;
	return myself;
}

/**
 * [Block] whileFalse: [block]
 *
 * Runs a block of code, depending on the outcome runs the other block
 * as long as the result of the first one equals to False.
 *
 * Usage:
 *
 * x := 0.
 * { ^(x > 5). }
 * whileFalse: { x add: 1. }. #increment x until it reaches 6.
 *
 * Here we increment variable x by one until it reaches 6.
 * While the number x is not higher than 5 we keep incrementing it.
 * Don't forget to use the return ^ symbol in the first block.
 */
ctr_object* ctr_block_while_false(ctr_object* myself, ctr_argument* argumentList) {
	while (1 && !CtrStdFlow) {
		ctr_object* result = ctr_internal_cast2bool(ctr_block_run(myself, argumentList, NULL));
		if (result->value.bvalue == 1 || CtrStdFlow) break;
		ctr_block_run(argumentList->object, argumentList, NULL);
		if (CtrStdFlow == CtrStdContinue) CtrStdFlow = NULL; /* consume continue */
	}
	if (CtrStdFlow == CtrStdBreak) CtrStdFlow = NULL; /* consume break */
	return myself;
}

/**
 * [Block] run
 *
 * Sending the unary message 'run' to a block will cause it to execute.
 * The run message takes no arguments, if you want to use the block as a function
 * and send arguments, consider using the applyTo-family of messages instead.
 * This message just simply runs the block of code without any arguments.
 * 
 * Usage:
 * 
 * { Pen write: 'Hello World'. } run. #prints 'Hello World'
 * 
 * The example above will run the code inside the block and display
 * the greeting.
 */
ctr_object* ctr_block_runIt(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* result;
	result = ctr_block_run(myself, argumentList, myself); /* here me/my refers to block itself not object - this allows closures. */
	if (CtrStdFlow == CtrStdBreak || CtrStdFlow == CtrStdContinue) CtrStdFlow = NULL; /* consume break */
	return result;
}

/**
 * [Block] set: [name] value: [object]
 *
 * Sets a variable in a block of code. This how you can get closure-like
 * functionality.
 *
 * Usage:
 *
 * shout := { Pen write: (my message + '!!!'). }.
 * shout set: 'message' value: 'hello'.
 * shout run.
 *
 * Here we assign a block to a variable named 'shout'.
 * We assign the string 'hello' to the variable 'message' inside the block.
 * When we invoke the block 'shout' by sending the run message without any
 * arguments it will display the string: 'hello!!!'.
 *
 * Similarly, you could use this technique to create a block that returns a
 * block that applies a formula (for instance simple multiplication) and then set the
 * multiplier to use in the formula. This way, you could create a block
 * building 'formula blocks'. This is how you implement use closures
 * in Citrine.
 */
ctr_object* ctr_block_set(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* key = ctr_internal_cast2string(argumentList->object);
	ctr_object* value = argumentList->next->object;
	ctr_internal_object_set_property(myself, key, value, 0);
	return myself;
}

/**
 * [Block] error: [object].
 *
 * Sets error flag on a block of code.
 * This will throw an error / exception.
 * You can attach an object to the error, for instance
 * an error message.
 *
 * Example:
 *
 * {
 *   thisBlock error: 'oops!'.
 * } catch: { :errorMessage
 *   Pen write: errorMessage.
 * }, run.
 */
ctr_object* ctr_block_error(ctr_object* myself, ctr_argument* argumentList) {
	CtrStdFlow = argumentList->object;
	CtrStdFlow->info.sticky = 1;
	return myself;
}

/**
 * [Block] catch: [otherBlock]
 *
 * Associates an error clause to a block.
 * If an error (exception) occurs within the block this block will be
 * executed.
 *
 * Usage:
 *
 * #Raise error on division by zero.
 * {
 *    var z := 4 / 0.
 * } catch: { :errorMessage
 *    Pen write: e, brk.
 * }, run.
 */
ctr_object* ctr_block_catch(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* catchBlock = argumentList->object;
	ctr_internal_object_delete_property(myself, ctr_build_string_from_cstring( "catch" ), 0 );
	ctr_internal_object_add_property(myself, ctr_build_string_from_cstring( "catch" ), catchBlock, 0 );
	return myself;
}

/**
 * [Block] toString
 *
 * Returns a string representation of the Block. This basic behavior, part
 * of any object will just return [Block]. Other objects typically override this
 * behavior with more useful implementations.
 */
ctr_object* ctr_block_to_string(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_string_from_cstring( "[Block]" );
}
