

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


#define O() (obj*) calloc(sizeof(obj), 1)
#define A() (args*) calloc(sizeof(args), 1)

