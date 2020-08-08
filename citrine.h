
#include "dictionary.h"
#include "msg.h"

#include <inttypes.h>
#include <stdlib.h>

/**
 * Version information
 */
#define CTR_VERSION "0.9.3"
#define CTR_VERSION_NUM 93

/**
 * Define the Citrine tokens 
 */
#define CTR_TOKEN_REF 1
#define CTR_TOKEN_QUOTE 2
#define CTR_TOKEN_NUMBER 3
#define CTR_TOKEN_PAROPEN 4
#define CTR_TOKEN_PARCLOSE 5
#define CTR_TOKEN_BLOCKOPEN 6
#define CTR_TOKEN_BLOCKCLOSE 7
#define CTR_TOKEN_COLON 8
#define CTR_TOKEN_DOT 9
#define CTR_TOKEN_CHAIN 10
#define CTR_TOKEN_BOOLEANYES 12
#define CTR_TOKEN_BOOLEANNO 13
#define CTR_TOKEN_NIL 14
#define CTR_TOKEN_ASSIGNMENT 15
#define CTR_TOKEN_RET 16
#define CTR_TOKEN_FIN 99

/**
 * Define the UTF8 byte patterns
 */
#define CTR_UTF8_BYTE1 192
#define CTR_UTF8_BYTE2 224
#define CTR_UTF8_BYTE3 240

/**
 * Define AST node memory footprints,
 * types of nodes in the AST.
 */
#define CTR_AST_NODE 1
#define CTR_AST_PROGRAM 3

/**
 * Define the Citrine node types for the
 * Abstract Syntax Tree (AST).
 */
#define CTR_AST_NODE_EXPRASSIGNMENT 51
#define CTR_AST_NODE_EXPRMESSAGE 52
#define CTR_AST_NODE_UNAMESSAGE 53
#define CTR_AST_NODE_BINMESSAGE 54
#define CTR_AST_NODE_KWMESSAGE 55
#define CTR_AST_NODE_LTRSTRING 56
#define CTR_AST_NODE_REFERENCE 57
#define CTR_AST_NODE_LTRNUM 58
#define CTR_AST_NODE_CODEBLOCK 59
#define CTR_AST_NODE_RETURNFROMBLOCK 60
#define CTR_AST_NODE_PARAMLIST 76
#define CTR_AST_NODE_INSTRLIST 77
#define CTR_AST_NODE_ENDOFPROGRAM 79
#define CTR_AST_NODE_NESTED 80
#define CTR_AST_NODE_LTRBOOLTRUE 81
#define CTR_AST_NODE_LTRBOOLFALSE 82
#define CTR_AST_NODE_LTRNIL 83
#define CTR_AST_NODE_PROGRAM 84

/**
 * Define the basic object types.
 * All objects in Citrine are 'normal' objects, however some
 * native objects (and external objects) have special memory requirements,
 * these are specified using the object types.
 */
#define CTR_OBJECT_TYPE_OTNIL 0
#define CTR_OBJECT_TYPE_OTBOOL 1
#define CTR_OBJECT_TYPE_OTNUMBER 2
#define CTR_OBJECT_TYPE_OTSTRING 3
#define CTR_OBJECT_TYPE_OTBLOCK 4
#define CTR_OBJECT_TYPE_OTOBJECT 5
#define CTR_OBJECT_TYPE_OTNATFUNC 6
#define CTR_OBJECT_TYPE_OTARRAY 7
#define CTR_OBJECT_TYPE_OTMISC 8
#define CTR_OBJECT_TYPE_OTEX 9

/**
 * Define the two types of properties of
 * objects.
 */
#define CTR_CATEGORY_PRIVATE_PROPERTY 0
#define CTR_CATEGORY_PUBLIC_PROPERTY 0 /* same, all properties are PRIVATE, except those in CtrStdWorld, this is just to avoid confusion */
#define CTR_CATEGORY_PUBLIC_METHOD 1

#define CTR_MAX_STEPS_LIMIT 2000

/**
 * Define basic types for Citrine
 */
typedef  unsigned int ctr_bool;
typedef  double       ctr_number;
typedef  char*        ctr_raw_string;

typedef  size_t ctr_size;

/**
 * Internal Citrine String
 */
struct ctr_string {
	char* value;
	ctr_size vlen;
};
typedef struct ctr_string ctr_string;


/**
 * Map 
 */
struct ctr_map {
	struct ctr_mapitem* head;
	int size;
};
typedef struct ctr_map ctr_map;

/**
 * Map item
 */
struct ctr_mapitem {
	uint64_t hashKey;
	struct ctr_object* key;
	struct ctr_object* value;
	struct ctr_mapitem* prev;
	struct ctr_mapitem* next;
};
typedef struct ctr_mapitem ctr_mapitem;

/**
 * Citrine Argument (internal, not accsible to users).
 */
struct ctr_argument {
	struct ctr_object* object;
	struct ctr_argument* next;
};
typedef struct ctr_argument ctr_argument;

/**
 * Root Object
 */
struct ctr_object {
	ctr_map* properties;
	ctr_map* methods;
	struct {
		unsigned int type: 4;
		unsigned int mark: 1;
		unsigned int sticky: 1;
		unsigned int chainMode: 1;
		unsigned int selfbind: 1;
	} info;
	struct ctr_object* link;
	union uvalue {
		ctr_bool bvalue;
		ctr_number nvalue;
		ctr_string* svalue;
		struct ctr_tnode* block;
		struct ctr_collection* avalue;
		struct ctr_resource* rvalue;
		struct ctr_object* (*fvalue) (struct ctr_object* myself, struct ctr_argument* argumentList);
	} value;
	struct ctr_object* gnext;
};
typedef struct ctr_object ctr_object;

/**
 * Citrine Resource
 */
struct ctr_resource {
	unsigned int type;
	void* ptr;
};
typedef struct ctr_resource ctr_resource;

/**
 * Array Structure
 */
struct ctr_collection {
	ctr_size length;
	ctr_size head;
	ctr_size tail;
	ctr_object** elements;
};
typedef struct ctr_collection ctr_collection;


/**
 * AST Node
 */
struct ctr_tnode {
	char type;
	char modifier;
	char* value;
	ctr_size vlen;
	struct ctr_tlistitem* nodes;
};
typedef struct ctr_tnode ctr_tnode;

/**
 * AST Node List
 */
struct ctr_tlistitem {
	ctr_tnode* node;	
	struct ctr_tlistitem* next;
};
typedef struct ctr_tlistitem ctr_tlistitem;

struct ctr_source_map {
	ctr_tnode* node;
	uint32_t line;
	struct ctr_source_map* next;
};
typedef struct ctr_source_map ctr_source_map;

ctr_source_map* ctr_source_map_head;
int ctr_source_mapping;

/**
 * Core Objects
 */
extern ctr_object* CtrStdWorld;
extern ctr_object* CtrStdObject;
extern ctr_object* CtrStdBlock;
extern ctr_object* CtrStdString;
extern ctr_object* CtrStdNumber;
extern ctr_object* CtrStdBool;
extern ctr_object* CtrStdBoolTrue;
extern ctr_object* CtrStdBoolFalse;
extern ctr_object* CtrStdConsole;
extern ctr_object* CtrStdNil;
extern ctr_object* CtrStdGC;
extern ctr_object* CtrStdMap;
extern ctr_object* CtrStdArray;
extern ctr_object* CtrStdFile;
extern ctr_object* CtrStdSystem;
extern ctr_object* CtrStdCommand;
extern ctr_object* CtrStdClock;
extern ctr_object* CtrStdFlow;
extern ctr_object* CtrStdBreak;
extern ctr_object* CtrStdContinue;
extern ctr_object* CtrStdExit;
extern ctr_object* ctr_first_object;

/**
 * Hashkey
 */
extern char CtrHashKey[16];

/**
 * CLI Arguments
 */
extern int ctr_argc;
extern char** ctr_argv;
extern int errstack;
extern int hascatch;

/**
 * Mode of Operation
 */
extern char* ctr_mode_input_file;
extern char* ctr_mode_dict_file;
extern char* ctr_mode_hfile1;
extern char* ctr_mode_hfile2;

/**
 * Lexer functions
 */
extern void 	ctr_clex_load(char* prg);
extern int 	ctr_clex_tok();
extern char*  ctr_clex_code_pointer();
extern char* 	ctr_clex_tok_value();
extern long    ctr_clex_tok_value_length();
extern void 	ctr_clex_putback();
extern char*	ctr_clex_readstr();
extern char*   ctr_clex_tok_describe( int token );
extern char* ctr_clex_keyword_me_icon;
extern char* ctr_clex_keyword_my_icon;
extern char* ctr_clex_keyword_var_icon;
extern char* ctr_clex_desc_tok_assignment;
extern ctr_size ctr_clex_keyword_my_icon_len;
extern ctr_size ctr_clex_keyword_var_icon_len;
extern ctr_size ctr_clex_keyword_eol_len;
extern ctr_size ctr_clex_keyword_num_sep_dec_len;
extern ctr_size ctr_clex_keyword_num_sep_tho_len;
extern ctr_size ctr_clex_keyword_qo_len;
extern ctr_size ctr_clex_keyword_qc_len;
extern ctr_size ctr_clex_keyword_assignment_len;
extern ctr_size ctr_clex_keyword_return_len;

extern void ctr_clex_set_ignore_modes( int ignore );

/**
 * Lexer properties
 */
extern ctr_size ctr_clex_len;
extern ctr_size ctr_program_length;
extern int ctr_clex_line_number;
extern char* ctr_eofcode;

/**
 * UTF-8 functions
 */
extern ctr_size getBytesUtf8(char* strval, long startByte, ctr_size lenUChar);
extern ctr_size ctr_getutf8len(char* strval, ctr_size max);
extern int ctr_utf8size(char c);

/**
 * Parser functions
 */
extern ctr_tnode* ctr_cparse_parse(char* prg, char* pathString);
extern ctr_tnode* ctr_cparse_expr(int mode);
extern ctr_tnode* ctr_cparse_ret();

/**
 * Translator functions
 */
extern void ctr_translate_program(char* prg, char* programPath);
extern void ctr_translate_generate_dicts(char* hfile1, char* hfile2);

/**
 * Abstract Tree Walker functions
 */
extern uint64_t    ctr_cwlk_subprogram;
extern ctr_object* ctr_cwlk_run(ctr_tnode* program);
extern ctr_object* ctr_cwlk_expr(ctr_tnode* node, char* wasReturn);
extern ctr_tnode* ctr_cparse_block();
extern ctr_tnode* ctr_cparse_create_node( int type );

/**
 * Internal World functions
 */
extern int ctr_in_message;
extern void        ctr_initialize_world();
extern char*       ctr_internal_memmem(char* haystack, long hlen, char* needle, long nlen, int reverse );
extern void        ctr_internal_object_add_property(ctr_object* owner, ctr_object* key, ctr_object* value, int m);
extern void        ctr_internal_object_set_property(ctr_object* owner, ctr_object* key, ctr_object* value, int is_method);
extern void        ctr_internal_object_delete_property(ctr_object* owner, ctr_object* key, int is_method);
extern ctr_object* ctr_internal_object_find_property(ctr_object* owner, ctr_object* key, int is_method);
extern uint64_t    ctr_internal_index_hash(ctr_object* key);
extern void        ctr_internal_object_add_property(ctr_object* owner, ctr_object* key, ctr_object* value, int m);
extern ctr_object* ctr_internal_cast2bool( ctr_object* o );
extern ctr_object* ctr_internal_cast2number(ctr_object* o);
extern ctr_object* ctr_internal_create_object(int type);
extern ctr_object* ctr_internal_cast2string( ctr_object* o );
extern void*       ctr_internal_plugin_find( ctr_object* key );
extern ctr_object* ctr_find(ctr_object* key);
extern ctr_object* ctr_find_in_my(ctr_object* key);
extern ctr_object* ctr_assign_value(ctr_object* key, ctr_object* val);
extern ctr_object* ctr_assign_value_to_my(ctr_object* key, ctr_object* val);
extern ctr_object* ctr_assign_value_to_local(ctr_object* key, ctr_object* val);
extern char*       ctr_internal_readf(char* file_name, uint64_t* size_allocated);
extern void        ctr_internal_debug_tree(ctr_tnode* ti, int indent);
extern ctr_object* ctr_send_message(ctr_object* receiver, char* message, long len, ctr_argument* argumentList);
extern void ctr_internal_create_func(ctr_object* o, ctr_object* key, ctr_object* (*func)( ctr_object*, ctr_argument* ) );

/**
 * Scoping functions
 */
extern void ctr_open_context();
extern void ctr_close_context();

/**
 * Global Scoping variables
 */
extern ctr_object* ctr_contexts[301];
extern int ctr_context_id;
extern ctr_tnode* ctr_callstack[301];
extern uint8_t ctr_callstack_index;

/**
 * Nil Interface
 */
extern ctr_object* ctr_nil_new(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_nil_is_nil(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_nil_to_string(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_nil_to_number(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_nil_to_boolean(ctr_object* myself, ctr_argument* argumentList);

/**
 * Object Interface
 */
extern ctr_object* ctr_object_make(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_object_equals(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_object_on_do(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_object_respond(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_object_type(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_object_is_nil(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_object_myself(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_object_do(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_object_done(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_object_message(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_object_if_false(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_object_if_true(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_object_learn_meaning(ctr_object* myself, ctr_argument* ctr_argumentList);
extern ctr_object* ctr_object_to_string(ctr_object* myself, ctr_argument* ctr_argumentList);
extern ctr_object* ctr_object_to_number(ctr_object* myself, ctr_argument* ctr_argumentList);
extern ctr_object* ctr_object_to_boolean(ctr_object* myself, ctr_argument* ctr_argumentList);
extern ctr_object* ctr_object_respond_and(ctr_object* myself, ctr_argument* ctr_argumentList);
extern ctr_object* ctr_object_respond_and_and(ctr_object* myself, ctr_argument* ctr_argumentList);
extern ctr_object* ctr_object_respond_and_and_and(ctr_object* myself, ctr_argument* ctr_argumentList);
extern ctr_object* ctr_object_case_do(ctr_object* myself, ctr_argument* ctr_argumentList);
extern ctr_object* ctr_object_to_code(ctr_object* myself, ctr_argument* ctr_argumentList);

/**
 * Boolean Interface
 */
extern ctr_object* ctr_bool_new(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_bool_if_true(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_bool_if_false(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_bool_and(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_bool_nor(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_bool_or(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_bool_eq(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_bool_neq(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_bool_to_number(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_bool_to_string(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_bool_not(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_bool_either_or(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_bool_break(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_bool_continue(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_bool_copy(ctr_object* myself, ctr_argument* argumentList);

/**
 * Number Interface
 */
extern ctr_object* ctr_number_new(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_add(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_inc(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_minus(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_dec(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_multiply(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_times(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_mul(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_divide(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_div(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_higherThan(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_higherEqThan(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_lowerThan(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_lowerEqThan(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_eq(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_neq(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_modulo(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_factorial(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_floor(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_ceil(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_round(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_abs(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_sqrt(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_pow(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_min(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_max(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_odd(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_even(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_to_string(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_to_string_flat(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_to_boolean(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_between(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_to_step_do(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_positive(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_negative(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_to_byte(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_qualify(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_qualification(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_respond_to(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_random(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_number_copy(ctr_object* myself, ctr_argument* argumentList);

/**
 * String Interface
 */
extern ctr_object* ctr_string_new(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_bytes(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_length(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_fromto(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_from_length(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_concat(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_append(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_eq(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_neq(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_trim(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_at(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_byte_at(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_index_of(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_last_index_of(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_replace_with(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_split(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_to_number(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_in_to_number(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_to_boolean(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_to_lower(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_to_upper(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_skip(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_find_pattern_do(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_find_pattern_options_do(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_contains_pattern(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_contains(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_hash_with_key(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_to_string( ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_string_eval( ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_string_quotes_escape( ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_string_characters( ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_string_to_byte_array( ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_string_append_byte(ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_string_compare(ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_string_before(ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_string_before_or_same(ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_string_after(ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_string_after_or_same(ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_string_fill_in(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_copy(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_string_to_code(ctr_object* myself, ctr_argument* ctr_argumentList);

/**
 * Block Interface
 */
extern ctr_object* ctr_block_new(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_block_runIt(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_block_set(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_block_error(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_block_catch(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_block_while_true(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_block_run(ctr_object* myself, ctr_argument* argList, ctr_object* my);
extern ctr_object* ctr_block_times(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_block_procedure(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_block_to_string(ctr_object* myself, ctr_argument* argumentList);

/**
 * Array Interface
 */
extern ctr_object* ctr_array_new(ctr_object* myclass, ctr_argument* argumentList);
extern ctr_object* ctr_array_new_and_push(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_type(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_push(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_unshift(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_shift(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_count(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_join(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_pop(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_get(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_sort(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_put(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_from_length(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_splice(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_add(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_map(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_min(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_max(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_to_string(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_fill(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_last(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_second_last(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_first(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_delete(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_combine(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_index_of(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_array_copy(ctr_object* myself, ctr_argument* argumentList);

/**
 * HashMap Interface
 */
extern ctr_object* ctr_map_new(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_map_type(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_map_put(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_map_get(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_map_count(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_map_each(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_map_to_string(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_map_delete(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_map_keys(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_map_values(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_map_has(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_map_key_value(ctr_object* myself, ctr_argument* argumentList);

/**
 * Console Interface
 */
extern ctr_object* ctr_console_write(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_console_brk(ctr_object* myself, ctr_argument* argumentList);

/**
 * File Interface
 */
extern ctr_object* ctr_file_new(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_file_path(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_file_to_string(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_file_read(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_file_write(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_file_append(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_file_exists(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_file_size(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_file_delete(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_file_include(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_file_open(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_file_close(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_file_read_bytes(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_file_write_bytes(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_file_seek(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_file_seek_rewind(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_file_seek_end(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_file_descriptor(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_file_lock_generic(ctr_object* myself, ctr_argument* argumentList, int lock);
extern ctr_object* ctr_file_lock(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_file_unlock(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_file_list(ctr_object* myself, ctr_argument* argumentList);



/**
 * Command Object Interface
 */
extern ctr_object* ctr_program_argument(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_program_num_of_args(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_program_waitforinput(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_program_input(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_program_get_env(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_program_set_env(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_program_exit(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_program_flush(ctr_object* myself, ctr_argument* ctr_argumentList);
extern ctr_object* ctr_program_forbid_shell(ctr_object* myself, ctr_argument* ctr_argumentList);
extern ctr_object* ctr_program_forbid_file_write(ctr_object* myself, ctr_argument* ctr_argumentList);
extern ctr_object* ctr_program_forbid_file_read(ctr_object* myself, ctr_argument* ctr_argumentList);
extern ctr_object* ctr_program_forbid_include(ctr_object* myself, ctr_argument* ctr_argumentList);
extern ctr_object* ctr_program_countdown(ctr_object* myself, ctr_argument* ctr_argumentList);
extern ctr_object* ctr_program_err(ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_program_shell(ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_program_include(ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_program_tonumber(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_program_tostring(ctr_object* myself, ctr_argument* argumentList);

/**
 * Clock Interface
 */
extern ctr_object* ctr_clock_change( ctr_object* myself, ctr_argument* argumentList, uint8_t forward );
extern ctr_object* ctr_clock_wait(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_clock_time(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_clock_new(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_clock_new_set(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_clock_like(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_clock_day(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_clock_month(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_clock_year(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_clock_hour(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_clock_minute(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_clock_second(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_clock_weekday(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_clock_yearday(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_clock_week(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_clock_set_day(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_clock_set_month(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_clock_set_year(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_clock_set_hour(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_clock_set_minute(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_clock_set_second(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_clock_get_time( ctr_object* myself, ctr_argument* argumentList, char part );
extern ctr_object* ctr_clock_set_time( ctr_object* myself, ctr_argument* argumentList, char part );
extern ctr_object* ctr_clock_set_zone( ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_clock_get_zone( ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_clock_to_string( ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_clock_format( ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_clock_add( ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_clock_subtract( ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_clock_to_number( ctr_object* myself, ctr_argument* argumentList );
extern ctr_object* ctr_clock_copy( ctr_object* myself, ctr_argument* argumentList );
extern void ctr_clock_init( ctr_object* clock );

/**
 * Garbage Collector Object Interface
 */
extern ctr_object* ctr_gc_collect(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_gc_setmode(ctr_object* myself, ctr_argument* argumentList);
extern ctr_object* ctr_gc_setmemlimit(ctr_object* myself, ctr_argument* argumentList);
extern void ctr_gc_sweep( int all );

/**
 * Global Garbage Collector variables
 */
extern int ctr_gc_dust_counter;
extern int ctr_gc_object_counter;
extern int ctr_gc_kept_counter;
extern int ctr_gc_sticky_counter;
extern int ctr_gc_mode;

extern uint64_t ctr_gc_alloc;
extern uint64_t ctr_gc_memlimit;

/**
 * Literal Constructors (internal only)
 */
extern ctr_object* ctr_build_empty_string();
extern ctr_object* ctr_build_string(char* object, long vlen);
extern ctr_object* ctr_build_block(ctr_tnode* node);
extern ctr_object* ctr_build_number(char* object);
extern ctr_object* ctr_build_number_from_string(char* fixedStr, ctr_size strLength, char international);
extern ctr_object* ctr_build_number_from_float(ctr_number floatNumber);
extern ctr_object* ctr_build_bool(int truth);
extern ctr_object* ctr_build_nil();
extern ctr_object* ctr_build_string_from_cstring( char* str );
extern void ctr_gc_internal_collect();
extern ctr_object* ctr_gc_internal_pin( ctr_object* object );
extern ctr_object* ctr_gc_memory(ctr_object* myself, ctr_argument* argumentList);


extern void* ctr_heap_allocate( size_t size );
extern void* ctr_heap_allocate_tracked( size_t size );
extern void  ctr_heap_free( void* ptr );
extern void  ctr_heap_free_rest();
extern void* ctr_heap_reallocate(void* oldptr, size_t size );
extern size_t ctr_heap_get_latest_tracking_id();
extern void* ctr_heap_reallocate_tracked(size_t tracking_id, size_t size );
extern char* ctr_heap_allocate_cstring( ctr_object* o );
extern ctr_object* ctr_error( char* error_string, int error_code );
extern ctr_object* ctr_error_text( char* error_string );
extern void ctr_pool_init( ctr_size pool );

extern char ctr_deserialize_mode;
extern char ctr_flag_sandbox;
extern uint16_t ctr_sandbox_steps;

extern char ctr_program_log_type;
extern void ctr_clex_move_code_pointer(int movement);
extern int ctr_clex_forward_scan( char* codePointer, ctr_size* newCodePointer );
extern int ctr_clex_backward_scan( char* codePointer, char* bytes, ctr_size* newCodePointer, ctr_size limit );
extern void ctr_print_error(char* error, int code);

extern void ctr_plugin_check_language( char* code );

