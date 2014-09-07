

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


#define CTR_CREATE_OBJECT() (obj*) calloc(sizeof(obj), 1)

#define CTR_CREATE_OBJECT_TYPE(O,C,V,OT) O = CTR_CREATE_OBJECT(); O->name = C; O->value = V; O->type = OT; HASH_ADD_KEYPTR(hh, World->properties, O->name, strlen(O->name), O);

#define CTR_CREATE_FUNC(X,Y,Z,Q) obj* X = CTR_CREATE_OBJECT();\
X->name = Z; X->type = OTNATFUNC; \
X->value = (void*) Y; \
HASH_ADD_KEYPTR(hh, Q->methods, X->name, strlen(X->name), X);

#define CTR_CREATE_ARGUMENT() (args*) calloc(sizeof(args), 1)

