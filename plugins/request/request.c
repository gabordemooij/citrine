#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include <unistd.h>
#include <syslog.h>
#include "../../citrine.h"

#include "ccgi-1.2/ccgi.h"


/* gcc -c request.c -Wall -Werror -fpic -o request.o ; gcc -shared -o libctrrequest.so request.o */

CGI_varlist *varlistGet;
CGI_varlist *varlistPost;
CGI_varlist *varlistCookie;
ctr_object* CtrStdSCGICB;


    
void begin (void) __attribute__((constructor));



/**
 * @internal
 * 
 * Returns a string from the request, either for GET, POST or COOKIE.
 */
ctr_object* ctr_request_string(ctr_object* myself, ctr_argument* argumentList, CGI_varlist* varlist) {
	ctr_object* cgiVarObject;
	char* cgiVar;
	char* value;
	cgiVarObject = ctr_internal_cast2string(argumentList->object);
	CTR_2CSTR(cgiVar, cgiVarObject);
	value = (char*) CGI_lookup(varlist, (const char*)cgiVar);
	if (value == NULL) return CtrStdNil;
	return ctr_build_string_from_cstring(value);
}

/**
 * @internal
 * 
 * Returns an array from the request, either for GET, POST or COOKIE.
 */
ctr_object* ctr_request_array(ctr_object* myself, ctr_argument* argumentList, CGI_varlist* varlist) {
	ctr_object* cgiVarObject;
	ctr_object* list;
	char* cgiVar;
	const CGI_value* value;
	char* val;
	ctr_argument* arg;
	int i = 0;
	list = ctr_array_new(CtrStdArray, NULL);	
	cgiVarObject = ctr_internal_cast2string(argumentList->object);
	CTR_2CSTR(cgiVar, cgiVarObject);
	value = CGI_lookup_all(varlist, (const char*)cgiVar);
	if (value == NULL) {
		return list;
	}
	for (i = 0; value[i] != 0; i++) {
		arg = CTR_CREATE_ARGUMENT();
		val = (char*) value[i];
		arg->object = ctr_build_string_from_cstring(val);
		ctr_array_push(list, arg);  
	}
	return list;
}

/**
 * @internal
 * 
 * Reads a server option set by user.
 */
ctr_object* ctr_request_internal_option(ctr_object* myself, char* optName) {
	ctr_object* key;
	ctr_object* val;
	key = ctr_build_string_from_cstring(optName);
	val = ctr_internal_object_find_property(myself, key, CTR_CATEGORY_PRIVATE_PROPERTY);
	return val;
}

/**
 * @internal
 *
 * callback for SCGI server. 
 */
void ctr_request_serve_callback() {
	ctr_argument* argumentList;
	argumentList = CTR_CREATE_ARGUMENT();
	varlistGet = CGI_get_query(NULL);
	varlistCookie = CGI_get_cookie(NULL);
	varlistPost = CGI_get_post(NULL,"/tmp/_upXXXXXX");
	ctr_block_run(CtrStdSCGICB, argumentList, CtrStdSCGICB);
	CGI_free_varlist(varlistGet);
	CGI_free_varlist(varlistPost);
	CGI_free_varlist(varlistCookie);
}

/**
 * Request get: [string]
 * 
 * Returns the value of the specified GET parameter from the HTTP query string.
 * For example if the query string of an url is: ?search=glasses
 * then the value of:
 * 
 * item := Request get: 'search'.
 * 
 * would be 'glasses'.
 */
ctr_object* ctr_request_get_string(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_request_string(myself, argumentList, varlistGet);
}

/**
 * Request getArray: [string].
 * 
 * Returns an array of strings extracted from the query string.
 * For example if the query string contains: ?option=a&option=b
 * 
 * Request getArray: 'option'.
 * 
 * will contain two elements: 'a' and 'b'. Note that
 * this also works with array-like notation: ?option[]='a'&option[]=b:
 * 
 * Request getArray: 'option[]'.
 * 
 * will return the same array.
 */
ctr_object* ctr_request_get_array(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_request_array(myself, argumentList, varlistGet);
}

/**
 * Request post: [string].
 * 
 * Obtains a string from the HTTP POST payload. Just like 'get:' but for
 * POST variables. See 'Request get:' for details.
 */
ctr_object* ctr_request_post_string(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_request_string(myself, argumentList, varlistPost);
}

/**
 * Request postArray: [string].
 * 
 * Obtains an array from the HTTP POST payload. Just like 'getArray:' but for
 * POST variables. See 'Request getArray:' for details.
 */
ctr_object* ctr_request_post_array(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_request_array(myself, argumentList, varlistPost);
}

/**
 * Request cookie: [string].
 * 
 * Obtains a string from the HTTP COOKIE payload. Just like 'get:' but for
 * COOKIE variables. See 'Request get:' for details.
 */
ctr_object* ctr_request_cookie_string(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_request_string(myself, argumentList, varlistCookie);
}

/**
 * Request cookieArray: [string].
 * 
 * Obtains an array from the HTTP COOKIE payload. Just like 'getArray:' but for
 * COOKIE variables. See 'Request getArray:' for details.
 */
ctr_object* ctr_request_cookie_array(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_request_array(myself, argumentList, varlistCookie);
}

/**
 * Request file: [string].
 * 
 * Returns array containing the path to the uploaded temporay file (0) and
 * the desired name of the uploaded file (1).
 */
ctr_object* ctr_request_file(ctr_object* myself, ctr_argument* argumentList) {
	CGI_value* value;
	ctr_object* list;
	ctr_object* cgiVarObject;
	char* cgiVar;
	char* val;
	ctr_argument* arg;
	int i = 0;
	cgiVarObject = ctr_internal_cast2string(argumentList->object);
	CTR_2CSTR(cgiVar, cgiVarObject);
	value = CGI_lookup_all(varlistPost, (const char*)cgiVar);
    list = ctr_array_new(CtrStdArray, NULL);
	if (value == 0 || value[1] == 0) return list;
    for (i = 0; value[i] != 0; i++) {
		arg = CTR_CREATE_ARGUMENT();
		val = (char*) value[i];
		arg->object = ctr_build_string_from_cstring(val);
		ctr_array_push(list, arg);
	}
	return list;
}

/**
 * Request serverOption: [string] is: [string].
 * 
 * Sets a server option, available server option for SCGI server include:
 * 
 * - minidle, minimum number of idle processes
 * - maxidle, maximum number of idle processes
 * - maxproc, maximum number of processes
 * - maxreq,  maximum number of concurrent requests to allow
 * 
 * Usage:
 * 
 * Request
 *  serverOption: 'minidle' is: 8,
 *  serverOption: 'maxreq'  is: 100.
 * 
 * This sets the minimum number of idle processes to 8 and the
 * maximum number of concurrent requests to 100, you can chain
 * multiple options using a comma (,).
 */
ctr_object* ctr_request_server_option(ctr_object* myself, ctr_argument* argumentList) {
	ctr_internal_object_set_property(
		myself,
		ctr_internal_cast2string(argumentList->object),
		ctr_internal_cast2string(argumentList->next->object),
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	return myself;
}

/**
 * Request host: [string] listen: [string] pid: [string] callback: [block].
 *
 * Sets up Storm Server.
 * Storm Server is an SCGI server. Both the Request Object Plugin and Storm Server
 * are based on S. Losen's CCGI library (http://libccgi.sourceforge.net/doc.html)
 * licensed LGPL.
 *
 * To set up a Storm Server, specify host (i.e. 'localhost'),
 * a port to listen to (i.e. 9000) a pid file '/var/run/mypid.pid' and a
 * callback block.
 *
 * Usage:
 *
 * Request host:'localhost' listen:4000 pid:'/var/run/storm.pid' callback: {\
 *  Pen write: 'Content-type: text/html\n\n'.
 *  var fname  := Command env: 'DOCUMENT_URI'.
 *  var script := File new: '/var/www/webapp'+fname.
 *  script include.
 * }.
 * 
 * Here we set up a server listening to port 4000. The callback prints out
 * the content type header. Then, we extract the DOCUMENT URI, i.e. '/hello.ctr'
 * and map this to a path '/var/www/webapp/hello.ctr'
 * 
 * By default there is no output buffering, either create another callback or
 * simply override the '<' or 'Pen' object to buffer instead of outputting
 * directly.
 */
ctr_object* ctr_request_serve(ctr_object* myself, ctr_argument* argumentList) {
	char* host;
	char* pid;
	int   port;
	int   minidle = 8;
	int   maxidle = 8;
	int   maxreq  = 1000;
	int   maxproc = 100;
	ctr_object* val;
	openlog("stormserver", 0, LOG_DAEMON);
	val = ctr_request_internal_option(myself, "minidle");
	if (val!=NULL) minidle = (int) ctr_internal_cast2number(val)->value.nvalue;
	val = ctr_request_internal_option(myself, "maxidle");
	if (val!=NULL) maxidle = (int) ctr_internal_cast2number(val)->value.nvalue;
	val = ctr_request_internal_option(myself, "maxproc");
	if (val!=NULL) maxproc = (int) ctr_internal_cast2number(val)->value.nvalue;
	val = ctr_request_internal_option(myself, "maxreq");
	if (val!=NULL) maxreq = (int) ctr_internal_cast2number(val)->value.nvalue;
	CTR_2CSTR(host, ctr_internal_cast2string(argumentList->object));
	CTR_2CSTR(pid, ctr_internal_cast2string(argumentList->next->next->object));
	port = (int) round(ctr_internal_cast2number(argumentList->next->object)->value.nvalue);
	CtrStdSCGICB = argumentList->next->next->next->object;
	CGI_prefork_server(host, port, pid,
        /* maxproc */ maxproc,
        /* minidle */ minidle,
        /* maxidle */ maxidle,
        /* maxreq */   maxreq, ctr_request_serve_callback);
    return myself;
}

/**
 * @internal
 *
 * Adds the Request object to the World.
 */
void begin(){
	ctr_object* requestObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	requestObject->link = CtrStdObject;
	ctr_internal_create_func(requestObject, ctr_build_string("get:", 4), &ctr_request_get_string);
	ctr_internal_create_func(requestObject, ctr_build_string("getArray:", 9), &ctr_request_get_array);
	ctr_internal_create_func(requestObject, ctr_build_string("cookie:", 7), &ctr_request_cookie_string);
	ctr_internal_create_func(requestObject, ctr_build_string("cookieArray:", 12), &ctr_request_cookie_array);
	ctr_internal_create_func(requestObject, ctr_build_string("post:", 5), &ctr_request_post_string);
	ctr_internal_create_func(requestObject, ctr_build_string("file:", 5), &ctr_request_file);
	ctr_internal_create_func(requestObject, ctr_build_string("postArray:", 10), &ctr_request_post_array);
	ctr_internal_create_func(requestObject, ctr_build_string("serverOption:is:", 16), &ctr_request_server_option);
	ctr_internal_create_func(requestObject, ctr_build_string("host:listen:pid:callback:", 25), &ctr_request_serve);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string("Request", 7), requestObject, 0);
	varlistGet = CGI_get_query(NULL);
	varlistPost = CGI_get_post(NULL,0);
	varlistCookie = CGI_get_cookie(NULL);
}
