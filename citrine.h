#define REF 1
#define QUOTE 2
#define NUMBER 3
#define PAROPEN 4
#define PARCLOSE 5
#define BLOCKOPEN 6
#define BLOCKCLOSE 7
#define COLON 8
#define DOT 9
#define CHAIN 10
#define BLOCKPIPE 11
#define BOOLEANYES 12
#define BOOLEANNO 13
#define NIL 14
#define ASSIGNMENT 15
#define RET 16
#define FIN 99

#define UTF8_BYTE1 192
#define UTF8_BYTE2 224
#define UTF8_BYTE3 240

#define EXPRASSIGNMENT 51
#define EXPRMESSAGE 52
#define UNAMESSAGE 53
#define BINMESSAGE 54
#define KWMESSAGE 55
#define LTRSTRING 56
#define REFERENCE 57
#define LTRNUM 58
#define CODEBLOCK 59
#define RETURNFROMBLOCK 60
#define PARAMLIST 76
#define INSTRLIST 77
#define ENDOFPROGRAM 79
#define NESTED 80
#define LTRBOOLTRUE 81
#define LTRBOOLFALSE 82
#define LTRNIL 83

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

struct cstr {
	char* value;
	long vlen;
};
struct cstr;
typedef struct cstr cstr;


/**
 * == Citrine Map ==
 * Represents a collection of objects,
 * can be extended to function like a hashmap.
 */
struct ctr_map {
	struct cmapitem* head;
	int size;
};
struct ctr_map;
typedef struct ctr_map ctr_map;

struct cmapitem {
	struct obj* key;
	struct obj* value;
	struct cmapitem* prev;
	struct cmapitem* next;
};
struct cmapitem;
typedef struct cmapitem cmapitem;


struct obj {
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
    struct obj* link;
    union uvalue {
		int bvalue;
		double nvalue;
		cstr* svalue;
		struct tnode* block;
		struct carray* avalue;
		struct cres* rvalue;
	} value;
	struct obj* gnext;
};
struct obj;
typedef struct obj obj;


struct cres {
	int type;
	void* ptr;
};
struct cres;
typedef struct cres cres;

struct carray {
	long length;
	long head;
	obj** elements;
};
struct carray;
typedef struct carray carray;

struct args {
	struct obj* object;
	struct args* next;
};

struct args;
typedef struct args args;

struct tnode {
	int type;
	int modifier;
	char* value;
	long vlen;
	struct tlistitem* nodes;
};


struct tlistitem {
	struct tnode* node;	
	struct tlistitem* next;
};

struct tlistitem;
typedef struct tlistitem tlistitem;

struct tnode;
typedef struct tnode tnode;

tnode* dparse_parse(char* prg);

long clex_len;
long ctr_program_length;

void 	clex_load(char* prg);
int 	clex_tok();
char* 	clex_tok_value();
long    clex_tok_value_length();
void 	clex_putback();
char*	clex_readstr();

void ctr_initialize_world();
obj* ctr_internal_create_object(int type);
obj* ctr_find(obj* key);
obj* ctr_find_in_my(obj* key);
obj* ctr_assign_value(obj* key, obj* val);
obj* ctr_assign_value_to_my(obj* key, obj* val);
obj* ctr_build_string(char* object, long vlen);
obj* ctr_build_block(tnode* node);
obj* ctr_build_number(char* object);
obj* ctr_build_bool(int truth);
obj* ctr_build_nil();
obj* ctr_build_string_from_cstring( char* str );

int __argc;
char** __argv;


obj* ctr_send_message(obj* receiver, char* message, long len, args* argumentList);
char* readf(char* file_name);
void tree(tnode* ti, int indent);

obj* cwlk_run(tnode* program);
obj* cwlk_expr(tnode* node);
obj* ctr_first_object;

int debug;

#define CTR_DBG(MSG, O)...


#define CTR_DEBUG_STR(X,Y,L) if (debug) {\
	char* b = calloc(sizeof(char),L);\
	memcpy(b, Y, L);\
	printf(X, b);\
}\

#define CTR_IS_DELIM(X) (X == '(' || X == ')' || X == '=' || X == ',' || X == '.' || X == '|' || X == ':' || X == ' ')
#define CTR_IS_NO_TOK(X)  X!='#' && X!='(' && X!=')' && X!='{' && X!='}' && X!='|' && X!='\\' && X!='.' && X!=',' && X!='=' && X!='^'  && X!= ':' && X!= '\''
#define CTR_CREATE_ARGUMENT() (args*) calloc(sizeof(args), 1)
#define CTR_PARSER_CREATE_LISTITEM() (tlistitem*) calloc(1, sizeof(tlistitem))
#define	CTR_PARSER_CREATE_NODE() (tnode*) calloc(1,sizeof(tnode))
#define ASSIGN_STRING(o,p,v,s) o->p = calloc(s,sizeof(char)); memcpy( (char*) o->p,v,s);

#define CTR_CONVFP(s,x){\
char *buf = calloc(100, sizeof(char));\
char *p;\
snprintf(buf, 99, "%.10f", x);\
p = buf + strlen(buf) - 1;\
while (*p == '0' && *p-- != '.');\
*(p+1) = '\0';\
if (*p == '.') *p = '\0';\
strcpy(s, buf);\
free (buf);\
}
