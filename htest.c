
#include <string.h>  /* strcpy */
#include <stdlib.h>  /* malloc */
#include <stdio.h>   /* printf */
#include "uthash.h"

struct obj {
    const char* name;
    struct obj* properties;           
    int type;
    void* value;
    UT_hash_handle hh;
};

struct obj;
typedef struct obj obj;

void do_something() {
	printf("Hi there!\n");
}

int main(int argc, char *argv[]) {
    
    obj* o = NULL;
    obj* root = NULL;
    obj* found = NULL;
    obj* found2 = NULL;  
    obj* method = NULL;
    
    o = (obj*)malloc(sizeof(obj));
    o->name = "Hello";
 	o->type = 3;
 	o->value = (void*) 3;
 
	method = (obj*)malloc(sizeof(obj));
	method->name = "greet";
	method->type = 4;
	method->value = (void*) &do_something;
	
	HASH_ADD_KEYPTR( hh, o->properties, method->name, strlen(method->name), method);
    HASH_ADD_KEYPTR( hh, root, o->name, strlen(o->name), o);
    
    HASH_FIND_STR(root, "Hello", found);
    printf("Found %s\n", found->name);
    
    HASH_FIND_STR(found->properties, "greet", found2);
    
    printf("Found %s\n", found2->name);
    
    void (*f)();
    f = found2->value;
    f();
    
    return 0;
    
    
    
}

