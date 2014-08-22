
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


#define LI() (tlistitem*) calloc(1, sizeof(tlistitem*))

#define	N() (tnode*) calloc(1,sizeof(tnode*))
			
#define TVAL(x) x->value = calloc(strlen(clex_tok_value()), sizeof(char)); strcpy(paramItem->value, clex_tok_value());
			
