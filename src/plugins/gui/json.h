#include "jsmn/jsmn.h"

ctr_object* ctr_string_escape(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_unescape(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_json_new(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_jsmn_dump( char* data, jsmntok_t** tt );
ctr_object* ctr_json_parse(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_json_jsonify(ctr_object* myself, ctr_argument* argumentList);
void begin_json();

