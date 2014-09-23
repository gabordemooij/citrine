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

#define OTNIL 80
#define OTBOOL 81
#define OTNUMBER 82
#define OTSTRING 83
#define OTBLOCK 84
#define OTOBJECT 85
#define OTNATFUNC 86
#define OTCUST 87
#define OTMISC 88
#define OTEX 89

struct obj {
    const char* name;
    struct obj* properties;
    struct obj* methods;
    int type;
    struct tnode* block;
    struct obj* link;
    char* value;
    UT_hash_handle hh;
};

struct obj;
typedef struct obj obj;

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

void 	clex_load(char* prg);
int 	clex_tok();
char* 	clex_tok_value();
void 	clex_putback();
char*	clex_readstr();

void ctr_initialize_world();
obj* ctr_find(char* key);
obj* ctr_find_in_my(char* key);
obj* ctr_assign_value(char* name, obj* object);
obj* ctr_assign_value_to_my(char* name, obj* object);
obj* ctr_build_string(char* object);
obj* ctr_build_block(tnode* node);
obj* ctr_build_number(char* object);
obj* ctr_build_bool(int truth);
obj* ctr_build_nil();

obj* ctr_send_message(obj* receiver, char* message, args* argumentList);
char* readf(char* file_name);
void tree(tnode* ti, int indent);

obj* cwlk_run(tnode* program);
obj* cwlk_expr(tnode* node);

int debug;

#define CTR_IS_DELIM(X) (X == '(' || X == ')' || X == '=' || X == ';' || X == '.' || X == '|' || X == ':' || X == ' ')
#define CTR_IS_NO_TOK(X)  X!='#' && X!='(' && X!=')' && X!='{' && X!='}' && X!='|' && X!='\\' && X!='.' && X!=';' && X!='=' && X!='^'  && X!= ':' && X!= '\''

#define CTR_CREATE_OBJECT() (obj*) calloc(sizeof(obj), 1)

#define CTR_CREATE_OBJECT_TYPE(O,C,V,OT) O = CTR_CREATE_OBJECT(); O->name = C; O->value = V; O->type = OT; HASH_ADD_KEYPTR(hh, World->properties, O->name, strlen(O->name), O);

#define CTR_CREATE_FUNC(X,Y,Z,Q) obj* X = CTR_CREATE_OBJECT();\
X->name = Z; X->type = OTNATFUNC; \
X->value = (void*) Y; \
HASH_ADD_KEYPTR(hh, Q->methods, X->name, strlen(X->name), X);

#define CTR_CREATE_ARGUMENT() (args*) calloc(sizeof(args), 1)

#define CTR_PARSER_CREATE_LISTITEM() (tlistitem*) calloc(1, sizeof(tlistitem*))

#define	CTR_PARSER_CREATE_NODE() (tnode*) calloc(1,sizeof(tnode*))
			
#define CTR_PARSER_GET_TOKVAL(x) x->value = calloc(strlen(clex_tok_value()), sizeof(char)); strcpy(paramItem->value, clex_tok_value());


