
#ifdef langNL
#include "i18n/nl/dictionarynl.h"
#else
#include "dictionary.h"
#endif

#include <inttypes.h>
#include <stdlib.h>

#ifdef __MINGW32__
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

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

/**
 * Security profile bit flags.
 */
#define CTR_SECPRO_NO_SHELL 1
#define CTR_SECPRO_NO_FILE_WRITE 2
#define CTR_SECPRO_NO_FILE_READ 4
#define CTR_SECPRO_NO_INCLUDE 8
#define CTR_SECPRO_COUNTDOWN 16
#define CTR_SECPRO_EVAL 32
#define CTR_SECPRO_FORK 64

#define CTR_ANSI_COLOR_RED     "\x1b[31m"
#define CTR_ANSI_COLOR_GREEN   "\x1b[32m"
#define CTR_ANSI_COLOR_YELLOW  "\x1b[33m"
#define CTR_ANSI_COLOR_BLUE    "\x1b[34m"
#define CTR_ANSI_COLOR_MAGENTA "\x1b[35m"
#define CTR_ANSI_COLOR_CYAN    "\x1b[36m"
#define CTR_ANSI_COLOR_RESET   "\x1b[0m"

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
		unsigned int remote: 1;
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
extern EXPORT ctr_object* CtrStdWorld;
extern EXPORT ctr_object* CtrStdObject;
extern EXPORT ctr_object* CtrStdBlock;
extern EXPORT ctr_object* CtrStdString;
extern EXPORT ctr_object* CtrStdNumber;
extern EXPORT ctr_object* CtrStdBool;
extern EXPORT ctr_object* CtrStdConsole;
extern EXPORT ctr_object* CtrStdNil;
extern EXPORT ctr_object* CtrStdGC;
extern EXPORT ctr_object* CtrStdMap;
extern EXPORT ctr_object* CtrStdArray;
extern EXPORT ctr_object* CtrStdFile;
extern EXPORT ctr_object* CtrStdSystem;
extern EXPORT ctr_object* CtrStdDice;
extern EXPORT ctr_object* CtrStdCommand;
extern EXPORT ctr_object* CtrStdSlurp;
extern EXPORT ctr_object* CtrStdShell;
extern EXPORT ctr_object* CtrStdClock;
extern EXPORT ctr_object* CtrStdFlow;
extern EXPORT ctr_object* CtrStdBreak;
extern EXPORT ctr_object* CtrStdContinue;
extern EXPORT ctr_object* CtrStdExit;
extern EXPORT ctr_object* ctr_first_object;

/**
 * Hashkey
 */
extern char CtrHashKey[16];

/**
 * CLI Arguments
 */
extern int ctr_argc;
extern char** ctr_argv;

/**
 * Mode of Operation
 */
char* ctr_mode_input_file;


/**
 * Lexer functions
 */
void 	ctr_clex_load(char* prg);
int 	ctr_clex_tok();
char* 	ctr_clex_tok_value();
long    ctr_clex_tok_value_length();
void 	ctr_clex_putback();
char*	ctr_clex_readstr();
char*   ctr_clex_tok_describe( int token );
char*   ctr_clex_keyword_var;
char*   ctr_clex_keyword_me;
char*   ctr_clex_keyword_my;
ctr_size ctr_clex_keyword_my_len;
ctr_size ctr_clex_keyword_var_len;
char* ctr_clex_keyword_me_icon;
char* ctr_clex_keyword_my_icon;
char* ctr_clex_keyword_var_icon;
ctr_size ctr_clex_keyword_my_icon_len;
ctr_size ctr_clex_keyword_var_icon_len;
ctr_size ctr_clex_string_interpolation_start_len;
ctr_size ctr_clex_string_interpolation_stop_len;


/**
 * Lexer properties
 */
ctr_size ctr_clex_len;
ctr_size ctr_program_length;
int ctr_clex_line_number;

/**
 * UTF-8 functions
 */
ctr_size getBytesUtf8(char* strval, long startByte, ctr_size lenUChar);
ctr_size ctr_getutf8len(char* strval, ctr_size max);
int ctr_utf8size(char c);

/**
 * Parser functions
 */
ctr_tnode* ctr_cparse_parse(char* prg, char* pathString);
ctr_tnode* ctr_cparse_expr(int mode);
ctr_tnode* ctr_cparse_ret();

/**
 * Abstract Tree Walker functions
 */
uint64_t    ctr_cwlk_subprogram;
ctr_object* ctr_cwlk_run(ctr_tnode* program);
ctr_object* ctr_cwlk_expr(ctr_tnode* node, char* wasReturn);

/**
 * Internal World functions
 */
EXPORT void        ctr_initialize_world();
EXPORT char*       ctr_internal_memmem(char* haystack, long hlen, char* needle, long nlen, int reverse );
EXPORT void        ctr_internal_object_add_property(ctr_object* owner, ctr_object* key, ctr_object* value, int m);
EXPORT void        ctr_internal_object_set_property(ctr_object* owner, ctr_object* key, ctr_object* value, int is_method);
EXPORT void        ctr_internal_object_delete_property(ctr_object* owner, ctr_object* key, int is_method);
EXPORT ctr_object* ctr_internal_object_find_property(ctr_object* owner, ctr_object* key, int is_method);
EXPORT uint64_t    ctr_internal_index_hash(ctr_object* key);
EXPORT void        ctr_internal_object_add_property(ctr_object* owner, ctr_object* key, ctr_object* value, int m);
EXPORT ctr_object* ctr_internal_cast2bool( ctr_object* o );
EXPORT ctr_object* ctr_internal_cast2number(ctr_object* o);
EXPORT ctr_object* ctr_internal_create_object(int type);
EXPORT ctr_object* ctr_internal_cast2string( ctr_object* o );
EXPORT void*       ctr_internal_plugin_find( ctr_object* key );
EXPORT ctr_object* ctr_find(ctr_object* key);
EXPORT ctr_object* ctr_find_in_my(ctr_object* key);
EXPORT ctr_object* ctr_assign_value(ctr_object* key, ctr_object* val);
EXPORT ctr_object* ctr_assign_value_to_my(ctr_object* key, ctr_object* val);
EXPORT ctr_object* ctr_assign_value_to_local(ctr_object* key, ctr_object* val);
EXPORT ctr_object* ctr_assign_value_to_local_by_ref(ctr_object* key, ctr_object* val);
EXPORT char*       ctr_internal_readf(char* file_name, uint64_t* size_allocated);
EXPORT void        ctr_internal_debug_tree(ctr_tnode* ti, int indent);
EXPORT ctr_object* ctr_send_message(ctr_object* receiver, char* message, long len, ctr_argument* argumentList);
EXPORT void ctr_internal_create_func(ctr_object* o, ctr_object* key, ctr_object* (*func)( ctr_object*, ctr_argument* ) );

/**
 * Scoping functions
 */
void ctr_open_context();
void ctr_close_context();

/**
 * Global Scoping variables
 */
ctr_object* ctr_contexts[300];
int ctr_context_id;
ctr_tnode* ctr_callstack[300];
uint8_t ctr_callstack_index;

/**
 * Nil Interface
 */
EXPORT ctr_object* ctr_nil_is_nil(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_nil_to_string(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_nil_to_number(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_nil_to_boolean(ctr_object* myself, ctr_argument* argumentList);

/**
 * Object Interface
 */
EXPORT ctr_object* ctr_object_make(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_object_equals(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_object_on_do(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_object_respond(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_object_type(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_object_is_nil(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_object_myself(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_object_do(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_object_done(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_object_message(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_object_if_false(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_object_if_true(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_object_learn_meaning(ctr_object* myself, ctr_argument* ctr_argumentList);
EXPORT ctr_object* ctr_object_to_string(ctr_object* myself, ctr_argument* ctr_argumentList);
EXPORT ctr_object* ctr_object_to_number(ctr_object* myself, ctr_argument* ctr_argumentList);
EXPORT ctr_object* ctr_object_to_boolean(ctr_object* myself, ctr_argument* ctr_argumentList);
EXPORT ctr_object* ctr_object_remote(ctr_object* myself, ctr_argument* ctr_argumentList);
EXPORT ctr_object* ctr_object_respond_and(ctr_object* myseld, ctr_argument* ctr_argumentList);
EXPORT ctr_object* ctr_object_respond_and_and(ctr_object* myseld, ctr_argument* ctr_argumentList);
EXPORT ctr_object* ctr_object_case_do(ctr_object* myseld, ctr_argument* ctr_argumentList);

/**
 * Boolean Interface
 */
EXPORT ctr_object* ctr_bool_if_true(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_bool_if_false(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_bool_and(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_bool_nor(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_bool_or(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_bool_eq(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_bool_neq(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_bool_xor(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_bool_to_number(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_bool_to_string(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_bool_not(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_bool_either_or(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_bool_break(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_bool_continue(ctr_object* myself, ctr_argument* argumentList);

/**
 * Number Interface
 */
EXPORT ctr_object* ctr_number_add(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_inc(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_minus(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_dec(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_multiply(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_times(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_mul(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_divide(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_div(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_higherThan(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_higherEqThan(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_lowerThan(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_lowerEqThan(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_eq(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_neq(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_modulo(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_factorial(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_floor(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_ceil(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_round(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_abs(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_sin(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_cos(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_exp(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_sqrt(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_tan(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_atan(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_log(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_pow(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_min(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_max(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_odd(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_even(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_to_string(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_to_boolean(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_between(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_to_step_do(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_positive(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_negative(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_to_byte(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_qualify(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_qualification(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_number_respond_to(ctr_object* myself, ctr_argument* argumentList);


/**
 * String Interface
 */
EXPORT ctr_object* ctr_string_bytes(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_length(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_fromto(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_from_length(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_concat(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_append(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_eq(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_neq(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_trim(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_ltrim(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_rtrim(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_padding_left(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_padding_right(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_html_escape(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_at(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_byte_at(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_index_of(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_last_index_of(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_replace_with(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_split(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_to_number(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_to_boolean(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_to_lower(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_to_upper(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_skip(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_to_lower1st(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_to_upper1st(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_find_pattern_do(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_find_pattern_options_do(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_contains_pattern(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_contains(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_hash_with_key(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_string_to_string( ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_string_eval( ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_string_quotes_escape( ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_string_characters( ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_string_to_byte_array( ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_string_append_byte(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_string_compare(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_string_before(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_string_before_or_same(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_string_after(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_string_after_or_same(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_string_escape(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_string_unescape(ctr_object* myself, ctr_argument* argumentList );

/**
 * Block Interface
 */
EXPORT ctr_object* ctr_block_runIt(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_block_set(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_block_error(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_block_catch(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_block_while_true(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_block_while_false(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_block_run(ctr_object* myself, ctr_argument* argList, ctr_object* my);
EXPORT ctr_object* ctr_block_times(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_block_to_string(ctr_object* myself, ctr_argument* argumentList);

/**
 * Array Interface
 */
EXPORT ctr_object* ctr_array_new(ctr_object* myclass, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_new_and_push(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_type(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_push(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_unshift(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_shift(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_count(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_join(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_pop(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_get(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_sort(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_put(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_from_length(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_splice(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_add(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_map(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_min(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_max(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_sum(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_product(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_to_string(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_fill(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_column(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_last(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_second_last(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_first(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_delete(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_combine(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_index_of(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_array_copy(ctr_object* myself, ctr_argument* argumentList);

/**
 * HashMap Interface
 */
EXPORT ctr_object* ctr_map_new(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_map_type(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_map_put(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_map_get(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_map_count(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_map_each(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_map_to_string(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_map_delete(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_map_keys(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_map_values(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_map_has(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_map_key_value(ctr_object* myself, ctr_argument* argumentList);

/**
 * Console Interface
 */
EXPORT ctr_object* ctr_console_write(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_console_brk(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_console_red(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_console_green(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_console_yellow(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_console_blue(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_console_magenta(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_console_cyan(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_console_reset(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_console_tab(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_console_line(ctr_object* myself, ctr_argument* argumentList);

/**
 * File Interface
 */
EXPORT ctr_object* ctr_file_new(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_path(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_to_string(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_read(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_write(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_append(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_exists(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_size(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_delete(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_include(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_open(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_close(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_read_bytes(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_write_bytes(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_seek(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_seek_rewind(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_seek_end(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_descriptor(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_lock_generic(ctr_object* myself, ctr_argument* argumentList, int lock);
EXPORT ctr_object* ctr_file_lock(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_unlock(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_list(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_file_temp_directory(ctr_object* myself, ctr_argument* argumentList); 

/**
 * Command Object Interface
 */
EXPORT ctr_object* ctr_program_OS(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_program_argument(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_program_num_of_args(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_program_waitforinput(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_program_input(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_program_get_env(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_program_set_env(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_program_exit(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_program_flush(ctr_object* myself, ctr_argument* ctr_argumentList);
EXPORT ctr_object* ctr_program_forbid_shell(ctr_object* myself, ctr_argument* ctr_argumentList);
EXPORT ctr_object* ctr_program_forbid_file_write(ctr_object* myself, ctr_argument* ctr_argumentList);
EXPORT ctr_object* ctr_program_forbid_file_read(ctr_object* myself, ctr_argument* ctr_argumentList);
EXPORT ctr_object* ctr_program_forbid_include(ctr_object* myself, ctr_argument* ctr_argumentList);
EXPORT ctr_object* ctr_program_forbid_fork(ctr_object* myself, ctr_argument* ctr_argumentList);
EXPORT ctr_object* ctr_program_countdown(ctr_object* myself, ctr_argument* ctr_argumentList);
EXPORT ctr_object* ctr_program_fork(ctr_object* myself, ctr_argument* ctr_argumentList);
EXPORT ctr_object* ctr_program_message(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_program_listen(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_program_join(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_program_log(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_program_warn(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_program_err(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_program_crit(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_program_pid(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_program_accept(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_program_accept_number(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_program_remote(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_program_default_port(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_program_shell(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_program_use_stderr(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_program_use_syslog(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_program_to_string(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_program_to_number(ctr_object* myself, ctr_argument* argumentList );

void ctr_check_permission( uint8_t operationID );
uint8_t ctr_program_security_profile;
uint64_t ctr_program_tick;
uint64_t ctr_program_maxtick;
ctr_object* (*ctr_secpro_eval_whitelist[64])(ctr_object*, ctr_argument*);

/**
 * Clock Interface
 */
EXPORT ctr_object* ctr_clock_change( ctr_object* myself, ctr_argument* argumentList, uint8_t forward );
EXPORT ctr_object* ctr_clock_wait(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_clock_time(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_clock_new(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_clock_new_set(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_clock_like(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_clock_day(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_clock_month(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_clock_year(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_clock_hour(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_clock_minute(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_clock_second(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_clock_weekday(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_clock_yearday(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_clock_week(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_clock_set_day(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_clock_set_month(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_clock_set_year(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_clock_set_hour(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_clock_set_minute(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_clock_set_second(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_clock_get_time( ctr_object* myself, ctr_argument* argumentList, char part );
EXPORT ctr_object* ctr_clock_set_time( ctr_object* myself, ctr_argument* argumentList, char part );
EXPORT ctr_object* ctr_clock_set_zone( ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_clock_get_zone( ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_clock_to_string( ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_clock_format( ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_clock_add( ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_clock_subtract( ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_clock_to_number( ctr_object* myself, ctr_argument* argumentList );
EXPORT void ctr_clock_init( ctr_object* clock );

/**
 * Slurp Interface
 */
EXPORT ctr_object* ctr_slurp_obtain(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_slurp_respond_to(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_slurp_respond_to_and(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_slurp_to_string(ctr_object* myself, ctr_argument* argumentList);

/**
 * Garbage Collector Object Interface
 */
EXPORT ctr_object* ctr_gc_collect(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_gc_dust(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_gc_object_count(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_gc_kept_count(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_gc_kept_alloc(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_gc_sticky_count(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_gc_setmode(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_gc_setmemlimit(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_gc_to_string(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_gc_to_number(ctr_object* myself, ctr_argument* argumentList);
EXPORT void ctr_gc_sweep( int all );

/**
 * Global Garbage Collector variables
 */
int ctr_gc_dust_counter;
int ctr_gc_object_counter;
int ctr_gc_kept_counter;
int ctr_gc_sticky_counter;
int ctr_gc_mode;

uint64_t ctr_gc_alloc;
uint64_t ctr_gc_memlimit;

/**
 * Misc Interfaces
 */
EXPORT ctr_object* ctr_dice_throw(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_dice_sides(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_dice_rand(ctr_object* myself, ctr_argument* argumentList);
EXPORT ctr_object* ctr_dice_randomize_bytes(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_dice_to_string(ctr_object* myself, ctr_argument* argumentList );
EXPORT ctr_object* ctr_dice_to_number(ctr_object* myself, ctr_argument* argumentList );

/**
 * Literal Constructors (internal only)
 */
EXPORT ctr_object* ctr_build_empty_string();
EXPORT ctr_object* ctr_build_string(char* object, long vlen);
EXPORT ctr_object* ctr_build_block(ctr_tnode* node);
EXPORT ctr_object* ctr_build_number(char* object);
EXPORT ctr_object* ctr_build_number_from_string(char* fixedStr, ctr_size strLength);
EXPORT ctr_object* ctr_build_number_from_float(ctr_number floatNumber);
EXPORT ctr_object* ctr_build_bool(int truth);
EXPORT ctr_object* ctr_build_nil();
EXPORT ctr_object* ctr_build_string_from_cstring( char* str );
EXPORT void ctr_gc_internal_collect();

EXPORT void* ctr_heap_allocate( size_t size );
EXPORT void* ctr_heap_allocate_tracked( size_t size );
EXPORT void  ctr_heap_free( void* ptr );
EXPORT void  ctr_heap_free_rest();
EXPORT void* ctr_heap_reallocate(void* oldptr, size_t size );
EXPORT size_t ctr_heap_get_latest_tracking_id();
EXPORT void* ctr_heap_reallocate_tracked(size_t tracking_id, size_t size );
EXPORT char* ctr_heap_allocate_cstring( ctr_object* o );
EXPORT ctr_object* ctr_error( char* error_string, int error_code );
EXPORT ctr_object* ctr_error_text( char* error_string );
EXPORT void ctr_pool_init( ctr_size pool );

uint8_t  ctr_accept_n_connections;
uint16_t ctr_default_port;

char ctr_program_log_type;
