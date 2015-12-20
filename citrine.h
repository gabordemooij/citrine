
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
 * THE CITRINE OBJECT.
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
 * Citrine Array
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
	unsigned int type;
	unsigned int modifier;
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
 * Generic Citrine variables
 */
ctr_size ctr_clex_len;
ctr_size ctr_program_length;
int ctr_argc;
char** ctr_argv;
int ctr_mode_debug;


/**
 * Generic Citrine Functions
 */
ctr_tnode* ctr_dparse_parse(char* prg);
void 	ctr_clex_load(char* prg);
int 	ctr_clex_tok();
char* 	ctr_clex_tok_value();
long    ctr_clex_tok_value_length();
void 	ctr_clex_putback();
char*	ctr_clex_readstr();
void ctr_initialize_world();
ctr_object* ctr_internal_create_object(int type);
ctr_object* ctr_internal_cast2string( ctr_object* o );
ctr_object* ctr_find(ctr_object* key);
ctr_object* ctr_find_in_my(ctr_object* key);
ctr_object* ctr_assign_value(ctr_object* key, ctr_object* val);
ctr_object* ctr_assign_value_to_my(ctr_object* key, ctr_object* val);
ctr_object* ctr_assign_value_to_local(ctr_object* key, ctr_object* val);
ctr_object* ctr_build_string(char* object, long vlen);
ctr_object* ctr_build_block(ctr_tnode* node);
ctr_object* ctr_build_number(char* object);
ctr_object* ctr_build_bool(int truth);
ctr_object* ctr_build_nil();
ctr_object* ctr_build_string_from_cstring( char* str );
ctr_object* ctr_block_run(ctr_object* myself, ctr_argument* argList, ctr_object* my);
ctr_size ctr_getutf8len(char* strval, ctr_size max);
ctr_object* ctr_send_message(ctr_object* receiver, char* message, long len, ctr_argument* argumentList);
char* ctr_internal_readf(char* file_name);
void ctr_internal_debug_tree(ctr_tnode* ti, int indent);
ctr_object* ctr_cwlk_run(ctr_tnode* program);
ctr_object* ctr_cwlk_expr(ctr_tnode* node, char* wasReturn);
ctr_object* ctr_first_object;

/**
 * Citrine Macros (temporary)
 */
#define CTR_DEBUG_STR(X,Y,L) if (ctr_mode_debug) {\
	char* b = calloc(sizeof(char),L);\
	memcpy(b, Y, L);\
	printf(X, b);\
}\

#define CTR_IS_DELIM(X) (X == '(' || X == ')' || X == ',' || X == '.' || X == '|' || X == ':' || X == ' ')
#define CTR_IS_NO_TOK(X)  X!='#' && X!='(' && X!=')' && X!='{' && X!='}' && X!='|' && X!='\\' && X!='.' && X!=',' && X!='^'  && X!= ':' && X!= '\''
#define CTR_CREATE_ARGUMENT() (ctr_argument*) calloc(sizeof(ctr_argument), 1)

char* xalloc(uintptr_t size, int what);
#define CTR_PARSER_CREATE_LISTITEM() (ctr_tlistitem*) xalloc(sizeof(ctr_tlistitem), 2)
#define	CTR_PARSER_CREATE_NODE() (ctr_tnode*) xalloc(sizeof(ctr_tnode), 1)


#define ASSIGN_STRING(o,p,v,s) o->p = xalloc(s * sizeof(char), 0); memcpy( (char*) o->p,v,s);

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

char* chunk;
uintptr_t chunk_ptr;
uintptr_t* abook;
