#include <inttypes.h>
#include <stdlib.h>

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
#define CTR_TOKEN_BLOCKPIPE 11
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
	const char* name;
	ctr_map* properties;
	ctr_map* methods;
	struct {
		unsigned int type: 4;
		unsigned int mark: 1;
		unsigned int sticky: 1;
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

/**
 * Core Objects
 */
ctr_object* CtrStdWorld;
ctr_object* CtrStdObject;
ctr_object* CtrStdBlock;
ctr_object* CtrStdString;
ctr_object* CtrStdNumber;
ctr_object* CtrStdBool;
ctr_object* CtrStdConsole;
ctr_object* CtrStdNil;
ctr_object* CtrStdGC;
ctr_object* CtrStdMap;
ctr_object* CtrStdArray;
ctr_object* CtrStdFile;
ctr_object* CtrStdError;
ctr_object* CtrStdSystem;
ctr_object* CtrStdDice;
ctr_object* CtrStdCommand;
ctr_object* CtrStdShell;
ctr_object* CtrStdClock;
ctr_object* CtrStdError;
ctr_object* CtrStdBreak;
ctr_object* CtrStdContinue;
ctr_object* ctr_first_object;

/**
 * Hashkey
 */
char CtrHashKey[16];

/**
 * CLI Arguments
 */
int ctr_argc;
char** ctr_argv;

/**
 * Mode of Operation
 */
char  ctr_mode_compile;
char  ctr_mode_debug;
char  ctr_mode_load;
char  ctr_mode_info;
char* ctr_mode_compile_save_as;
char* ctr_mode_input_file;


/**
 * Memory Management functions
 */
char* ctr_malloc(uintptr_t size, int what);
void* ctr_realloc(void* oldptr, uintptr_t size, uintptr_t old_size, int what);

/**
 * Memory Management variables
 */
char*      ctr_malloc_chunk;
uintptr_t  ctr_malloc_chunk_pointer;
uintptr_t* ctr_malloc_swizzle_adressbook;
int        ctr_malloc_mode;
uint64_t   ctr_malloc_measured_size_addressbook;
uint64_t   ctr_malloc_measured_size_code;


/**
 * AST header
 */
struct ctr_ast_header {
	char      version[10];          /* version */
	uint64_t  num_of_swizzles;      /* number of pointer swizzles */
	uint64_t  size_of_address_book; /* size of the addressbook for swizzl'n - should be size_t ? */
	uintptr_t start_block;          /* old pointer, start of the original memory block */
	uintptr_t program_entry_point;  /* original start address of program */
};
typedef struct ctr_ast_header ctr_ast_header;
ctr_ast_header* ctr_default_header;

/**
 * Serializer functions
 */
ctr_tnode* ctr_serializer_unserialize();
void ctr_serializer_serialize(ctr_tnode* t);
void ctr_serializer_info(char* filename);

/**
 * Lexer functions
 */
void 	ctr_clex_load(char* prg);
int 	ctr_clex_tok();
char* 	ctr_clex_tok_value();
long    ctr_clex_tok_value_length();
void 	ctr_clex_putback();
char*	ctr_clex_readstr();

/**
 * Lexer properties
 */
ctr_size ctr_clex_len;
ctr_size ctr_program_length;

/**
 * UTF-8 functions
 */
ctr_size getBytesUtf8(char* strval, long startByte, ctr_size lenUChar);
ctr_size ctr_getutf8len(char* strval, ctr_size max);
int ctr_utf8size(char c);

/**
 * Parser functions
 */
ctr_tnode* ctr_dparse_parse(char* prg);
ctr_tnode* ctr_cparse_expr(int mode);
ctr_tnode* ctr_cparse_ret();

/**
 * Abstract Tree Walker functions
 */
ctr_object* ctr_cwlk_run(ctr_tnode* program);
ctr_object* ctr_cwlk_expr(ctr_tnode* node, char* wasReturn);

/**
 * Internal World functions
 */
void        ctr_initialize_world();
char*       ctr_internal_memmem(char* haystack, long hlen, char* needle, long nlen, int reverse );
void        ctr_internal_object_add_property(ctr_object* owner, ctr_object* key, ctr_object* value, int m);
void        ctr_internal_object_set_property(ctr_object* owner, ctr_object* key, ctr_object* value, int is_method);
void        ctr_internal_object_delete_property(ctr_object* owner, ctr_object* key, int is_method);
ctr_object* ctr_internal_object_find_property(ctr_object* owner, ctr_object* key, int is_method);
uint64_t    ctr_internal_index_hash(ctr_object* key);
void        ctr_internal_object_add_property(ctr_object* owner, ctr_object* key, ctr_object* value, int m);
ctr_object* ctr_internal_cast2bool( ctr_object* o );
ctr_object* ctr_internal_cast2number(ctr_object* o);
ctr_object* ctr_internal_create_object(int type);
ctr_object* ctr_internal_cast2string( ctr_object* o );
void*       ctr_internal_plugin_find( ctr_object* key );
ctr_object* ctr_find(ctr_object* key);
ctr_object* ctr_find_in_my(ctr_object* key);
ctr_object* ctr_assign_value(ctr_object* key, ctr_object* val);
ctr_object* ctr_assign_value_to_my(ctr_object* key, ctr_object* val);
ctr_object* ctr_assign_value_to_local(ctr_object* key, ctr_object* val);
char*       ctr_internal_readf(char* file_name);
void        ctr_internal_debug_tree(ctr_tnode* ti, int indent);
ctr_object* ctr_send_message(ctr_object* receiver, char* message, long len, ctr_argument* argumentList);
void ctr_internal_create_func(ctr_object* o, ctr_object* key, ctr_object* (*func)( ctr_object*, ctr_argument* ) );

/**
 * Scoping functions
 */
void ctr_open_context();
void ctr_close_context();

/**
 * Global Scoping variables
 */
ctr_object* ctr_contexts[100];
int ctr_context_id;

/**
 * Nil Interface
 */
ctr_object* ctr_nil_is_nil(ctr_object* myself, ctr_argument* argumentList);

/**
 * Object Interface
 */
ctr_object* ctr_object_make(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_equals(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_on_do(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_respond(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_type(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_is_nil(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_object_myself(ctr_object* myself, ctr_argument* argumentList);

/**
 * Boolean Interface
 */
ctr_object* ctr_bool_iftrue(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_ifFalse(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_and(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_nor(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_or(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_eq(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_neq(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_xor(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_to_number(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_to_string(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_not(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_flip(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_either_or(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_break(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_bool_continue(ctr_object* myself, ctr_argument* argumentList);

/**
 * Number Interface
 */
ctr_object* ctr_number_add(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_inc(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_minus(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_dec(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_multiply(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_times(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_mul(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_divide(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_div(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_higherThan(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_higherEqThan(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_lowerThan(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_lowerEqThan(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_eq(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_neq(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_modulo(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_factorial(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_floor(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_ceil(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_round(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_abs(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_sin(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_cos(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_exp(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_sqrt(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_tan(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_atan(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_log(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_pow(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_min(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_max(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_odd(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_even(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_to_string(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_to_boolean(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_between(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_to_by_do(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_positive(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_number_negative(ctr_object* myself, ctr_argument* argumentList);

/**
 * String Interface
 */
ctr_object* ctr_string_bytes(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_length(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_fromto(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_from_length(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_concat(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_append(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_eq(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_neq(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_trim(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_ltrim(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_rtrim(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_html_escape(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_at(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_byte_at(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_index_of(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_last_index_of(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_replace_with(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_split(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_to_number(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_to_boolean(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_to_lower(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_to_upper(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_skip(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_to_lower1st(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_string_to_upper1st(ctr_object* myself, ctr_argument* argumentList);

/**
 * Block Interface
 */
ctr_object* ctr_block_runIt(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_block_set(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_block_error(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_block_catch(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_block_while_true(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_block_while_false(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_block_run(ctr_object* myself, ctr_argument* argList, ctr_object* my);
ctr_object* ctr_block_times(ctr_object* myself, ctr_argument* argumentList);

/**
 * Array Interface
 */
ctr_object* ctr_array_new(ctr_object* myclass, ctr_argument* argumentList);
ctr_object* ctr_array_new_and_push(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_push(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_unshift(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_shift(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_count(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_join(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_pop(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_get(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_sort(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_put(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_from_to(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_add(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_array_map(ctr_object* myself, ctr_argument* argumentList);

/**
 * HashMap Interface
 */
ctr_object* ctr_map_new(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_put(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_get(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_count(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_map_each(ctr_object* myself, ctr_argument* argumentList);

/**
 * Console Interface
 */
ctr_object* ctr_console_write(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_console_brk(ctr_object* myself, ctr_argument* argumentList);

/**
 * File Interface
 */
ctr_object* ctr_file_new(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_path(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_read(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_write(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_append(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_exists(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_size(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_delete(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_include(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_include_ast(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_open(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_close(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_read_bytes(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_write_bytes(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_seek(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_seek_rewind(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_file_seek_end(ctr_object* myself, ctr_argument* argumentList);


/**
 * Command Object Interface
 */
ctr_object* ctr_command_argument(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_command_num_of_args(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_command_question(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_command_get_env(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_command_set_env(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_command_exit(ctr_object* myself, ctr_argument* argumentList);

/**
 * Clock Interface
 */
ctr_object* ctr_clock_wait(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_clock_time(ctr_object* myself, ctr_argument* argumentList);

/**
 * Shell Interface
 */
ctr_object* ctr_shell_call(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_shell_respond_to(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_shell_respond_to_with(ctr_object* myself, ctr_argument* argumentList);

/**
 * Garbage Collector Object Interface
 */
ctr_object* ctr_gc_collect(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gc_dust(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_gc_object_count(ctr_object* myself, ctr_argument* argumentList);

/**
 * Global Garbage Collector variables
 */
int ctr_gc_dust_counter;
int ctr_gc_object_counter;


/**
 * Misc Interfaces
 */
ctr_object* ctr_dice_throw(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_dice_sides(ctr_object* myself, ctr_argument* argumentList);
ctr_object* ctr_dice_rand(ctr_object* myself, ctr_argument* argumentList);

/**
 * Literal Constructors (internal only)
 */
ctr_object* ctr_build_string(char* object, long vlen);
ctr_object* ctr_build_block(ctr_tnode* node);
ctr_object* ctr_build_number(char* object);
ctr_object* ctr_build_number_from_string(char* fixedStr, ctr_size strLength);
ctr_object* ctr_build_number_from_float(ctr_number floatNumber);
ctr_object* ctr_build_bool(int truth);
ctr_object* ctr_build_nil();
ctr_object* ctr_build_string_from_cstring( char* str );


/**
 * Citrine Macros
 */
#define CTR_DEBUG_STR(X,Y,L) if (ctr_mode_debug) {\
	char* b = calloc(sizeof(char),L);\
	memcpy(b, Y, L);\
	printf(X, b);\
}\

#define CTR_IS_DELIM(X) (X == '(' || X == ')' || X == ',' || X == '.' || X == '|' || X == ':' || X == ' ')
#define CTR_IS_NO_TOK(X)  X!='#' && X!='(' && X!=')' && X!='{' && X!='}' && X!='|' && X!='\\' && X!='.' && X!=',' && X!='^'  && X!= ':' && X!= '\''
#define CTR_CREATE_ARGUMENT() (ctr_argument*) calloc(sizeof(ctr_argument), 1)
#define CTR_PARSER_CREATE_LISTITEM() (ctr_tlistitem*) ctr_malloc(sizeof(ctr_tlistitem), 2)
#define	CTR_PARSER_CREATE_NODE() (ctr_tnode*) ctr_malloc(sizeof(ctr_tnode), 1)
#define	CTR_PARSER_CREATE_PROGRAM_NODE() (ctr_tnode*) ctr_malloc(sizeof(ctr_tnode), 3)
#define ASSIGN_STRING(o,p,v,s) o->p = ctr_malloc(s * sizeof(char), 0); memcpy( (char*) o->p,v,s);
#define CTR_2CSTR(cs, s) cs = ctr_malloc((s->value.svalue->vlen+1) * sizeof(char),0); strncpy(cs, s->value.svalue->value, s->value.svalue->vlen); cs[s->value.svalue->vlen] = '\0';

#define CTR_CONVFP(s,x){\
char *buf = calloc(100, sizeof(char));\
char *p;\
snprintf(buf, 99, "%.10f", x);\
p = buf + strlen(buf) - 1;\
while (*p == '0' && *p-- != '.');\
*(p+1) = '\0';\
if (*p == '.') *p = '\0';\
strncpy(s, buf, strlen(buf));\
free (buf);\
}


