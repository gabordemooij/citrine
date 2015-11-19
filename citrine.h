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

#define CTR_UTF8_BYTE1 192
#define CTR_UTF8_BYTE2 224
#define CTR_UTF8_BYTE3 240

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

#define OTNIL 0
#define OTBOOL 1
#define OTNUMBER 2
#define OTSTRING 3
#define OTBLOCK 4
#define OTOBJECT 5
#define OTNATFUNC 6
#define OTARRAY 7
#define OTMISC 8
#define OTEX 9

typedef  unsigned int ctr_bool;
typedef  long double ctr_number;
typedef  char* ctr_raw_string;
typedef  size_t ctr_size;

struct ctr_string {
	char* value;
	long vlen;
};
struct ctr_string;
typedef struct ctr_string ctr_string;


/**
 * == Citrine Map ==
 * Represents a collection of objects,
 * can be extended to function like a hashmap.
 */
struct ctr_map {
	struct ctr_mapitem* head;
	int size;
};
struct ctr_map;
typedef struct ctr_map ctr_map;

struct ctr_mapitem {
	struct ctr_object* key;
	struct ctr_object* value;
	struct ctr_mapitem* prev;
	struct ctr_mapitem* next;
};
struct ctr_mapitem;
typedef struct ctr_mapitem ctr_mapitem;


struct ctr_object {
    const char* name;
    ctr_map* properties;
    ctr_map* methods;
	 struct {
		unsigned int type: 4;
		unsigned int mark: 1;
		unsigned int sticky: 1;
		unsigned int flaga: 1;
		unsigned int flagb: 1;
	 } info;
    struct ctr_object* link;
    union uvalue {
		int bvalue;
		double nvalue;
		ctr_string* svalue;
		struct tnode* block;
		struct ctr_collection* avalue;
		struct ctr_resource* rvalue;
	} value;
	struct ctr_object* gnext;
};
struct ctr_object;
typedef struct ctr_object ctr_object;


struct ctr_resource {
	int type;
	void* ptr;
};
struct ctr_resource;
typedef struct ctr_resource ctr_resource;

struct ctr_collection {
	long length;
	long head;
	long tail;
	ctr_object** elements;
};
struct ctr_collection;
typedef struct ctr_collection ctr_collection;

struct ctr_argument {
	struct ctr_object* object;
	struct ctr_argument* next;
};

struct ctr_argument;
typedef struct ctr_argument ctr_argument;

struct ctr_tnode {
	int type;
	int modifier;
	char* value;
	long vlen;
	struct ctr_tlistitem* nodes;
};


struct ctr_tlistitem {
	struct ctr_tnode* node;	
	struct ctr_tlistitem* next;
};

struct ctr_tlistitem;
typedef struct ctr_tlistitem ctr_tlistitem;

struct ctr_tnode;
typedef struct ctr_tnode ctr_tnode;

ctr_tnode* dparse_parse(char* prg);

long clex_len;
long ctr_program_length;

void 	clex_load(char* prg);
int 	clex_tok();
char* 	clex_tok_value();
long    clex_tok_value_length();
void 	clex_putback();
char*	clex_readstr();

void ctr_initialize_world();
ctr_object* ctr_internal_create_object(int type);
ctr_object* ctr_find(ctr_object* key);
ctr_object* ctr_find_in_my(ctr_object* key);
ctr_object* ctr_assign_value(ctr_object* key, ctr_object* val);
ctr_object* ctr_assign_value_to_my(ctr_object* key, ctr_object* val);
ctr_object* ctr_build_string(char* object, long vlen);
ctr_object* ctr_build_block(ctr_tnode* node);
ctr_object* ctr_build_number(char* object);
ctr_object* ctr_build_bool(int truth);
ctr_object* ctr_build_nil();
ctr_object* ctr_build_string_from_cstring( char* str );
ctr_object* ctr_block_run(ctr_object* myself, ctr_argument* argList, ctr_object* my);
long getutf8len(char* strval, long max);

int __argc;
char** __argv;


ctr_object* ctr_send_message(ctr_object* receiver, char* message, long len, ctr_argument* argumentList);
char* readf(char* file_name);
void tree(ctr_tnode* ti, int indent);

ctr_object* cwlk_run(ctr_tnode* program);
ctr_object* cwlk_expr(ctr_tnode* node);
ctr_object* ctr_first_object;

int debug;


#define CTR_DEBUG_STR(X,Y,L) if (debug) {\
	char* b = calloc(sizeof(char),L);\
	memcpy(b, Y, L);\
	printf(X, b);\
}\

#define CTR_IS_DELIM(X) (X == '(' || X == ')' || X == ',' || X == '.' || X == '|' || X == ':' || X == ' ')
#define CTR_IS_NO_TOK(X)  X!='#' && X!='(' && X!=')' && X!='{' && X!='}' && X!='|' && X!='\\' && X!='.' && X!=',' && X!='^'  && X!= ':' && X!= '\''
#define CTR_CREATE_ARGUMENT() (ctr_argument*) calloc(sizeof(ctr_argument), 1)
#define CTR_PARSER_CREATE_LISTITEM() (ctr_tlistitem*) calloc(1, sizeof(ctr_tlistitem))
#define	CTR_PARSER_CREATE_NODE() (ctr_tnode*) calloc(1,sizeof(ctr_tnode))
#define ASSIGN_STRING(o,p,v,s) o->p = calloc(s,sizeof(char)); memcpy( (char*) o->p,v,s);

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
