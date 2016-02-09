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


ctr_object* ctr_request_get_string(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_request_string(myself, argumentList, varlistGet);
}

ctr_object* ctr_request_get_array(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_request_array(myself, argumentList, varlistGet);
}

ctr_object* ctr_request_post_string(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_request_string(myself, argumentList, varlistPost);
}

ctr_object* ctr_request_post_array(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_request_array(myself, argumentList, varlistPost);
}

ctr_object* ctr_request_cookie_string(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_request_string(myself, argumentList, varlistCookie);
}

ctr_object* ctr_request_cookie_array(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_request_array(myself, argumentList, varlistCookie);
}

ctr_object* ctr_request_response_header(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* headStrObj = ctr_internal_cast2string(argumentList->object);
	char* headerStr;
	CTR_2CSTR(headerStr, headStrObj);
	fputs(headerStr, stdout);
	return myself;
}

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

ctr_object* ctr_request_serve(ctr_object* myself, ctr_argument* argumentList) {
	char* host;
	char* pid;
	int   port;
	openlog("stormserver", 0, LOG_DAEMON);
	CTR_2CSTR(host, ctr_internal_cast2string(argumentList->object));
	CTR_2CSTR(pid, ctr_internal_cast2string(argumentList->next->next->object));
	port = (int) round(ctr_internal_cast2number(argumentList->next->object)->value.nvalue);
	CtrStdSCGICB = argumentList->next->next->next->object;
	CGI_prefork_server(host, port, pid,
        /* maxproc */ 100,
        /* minidle */ 8,
        /* maxidle */ 8,
        /* maxreq */ 1000, ctr_request_serve_callback);
    return myself;
}


void begin(){
	ctr_object* requestObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	requestObject->link = CtrStdObject;
	ctr_internal_create_func(requestObject, ctr_build_string("get:", 4), &ctr_request_get_string);
	ctr_internal_create_func(requestObject, ctr_build_string("getArray:", 9), &ctr_request_get_array);
	ctr_internal_create_func(requestObject, ctr_build_string("cookie:", 7), &ctr_request_cookie_string);
	ctr_internal_create_func(requestObject, ctr_build_string("cookieArray:", 12), &ctr_request_cookie_array);
	ctr_internal_create_func(requestObject, ctr_build_string("post:", 5), &ctr_request_post_string);
	ctr_internal_create_func(requestObject, ctr_build_string("file:", 5), &ctr_request_file);
	ctr_internal_create_func(requestObject, ctr_build_string("responseHeader:", 15), &ctr_request_response_header);
	ctr_internal_create_func(requestObject, ctr_build_string("postArray:", 10), &ctr_request_post_array);
	ctr_internal_create_func(requestObject, ctr_build_string("host:listen:pid:callback:", 25), &ctr_request_serve);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string("Request", 7), requestObject, 0);
	varlistGet = CGI_get_query(NULL);
	varlistPost = CGI_get_post(NULL,0);
}
