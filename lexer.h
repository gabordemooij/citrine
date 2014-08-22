
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

void 	clex_load(char* prg);
int 	clex_tok();
char* 	clex_tok_value();
void 	clex_putback();
char*	clex_readstr();
