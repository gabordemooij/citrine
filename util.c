#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include <unistd.h>
#include <dlfcn.h>
#include "citrine.h"

char* np;

/**
 * Determines the size of the specified file.
 */
int fsize(char* filename) {
  int size;
  FILE* fh;
  fh = fopen(filename, "rb");
  if(fh != NULL){
    if( fseek(fh, 0, SEEK_END) ){
      fclose(fh);
      return -1;
    }
    size = ftell(fh);
    fclose(fh);
    return size;
  }
  return -1;
}
/**
 * Serializer Serialize
 * Serializes an pre-aligned abstract syntax tree along with
 * its addressbook.
 */
void ctr_serializer_serialize(ctr_tnode* t) {
	FILE *f;
	f = fopen(ctr_mode_compile_save_as,"wb");
    if (!f) { printf("Unable to open file!"); exit(1); }
	memcpy( ctr_malloc_chunk, ctr_default_header, sizeof(ctr_ast_header));/* append to addressbook, new header */
	fwrite(ctr_malloc_chunk, sizeof(char), ctr_malloc_measured_size_code+ctr_malloc_measured_size_addressbook, f);
	fclose(f);
}

/**
 * Serializer Show Information
 * Outputs information about the specified AST file.
 */
void ctr_serializer_info(char* filename) {
	FILE *f;
	char* str;
	size_t s = 0;
	s = fsize(filename);
	np = calloc(sizeof(char),s);
	if (!np) { printf("no memory.\n"); exit(1);}
	f = fopen(filename,"rb");
	fread(np, sizeof(char), s, f);
	fclose(f);
	ctr_default_header = (ctr_ast_header*) (np);
	printf("Citrine AST-FILE INFO:\n");
	str = (char*) malloc(sizeof(char) * 11);
	*(str + 10) = '\0';
	strncpy(str, ctr_default_header->version, 10);
	printf("Version and Identification Code : %s \n", ctr_default_header->version);
	printf("Number of Pointer Swizzles      : %lu \n", (long) ctr_default_header->num_of_swizzles);
	printf("Size of Addressbook             : %lu \n", (long) ctr_default_header->size_of_address_book);
	printf("Start Original Memory Block (OB): %p \n", (char*) ctr_default_header->start_block);
	printf("Program Entry Point (PEP)       : %p \n", (char*) ctr_default_header->program_entry_point);
}

/**
 * Serializer Unserialize
 * Loads an AST file in memory and recalculates (unswizzles) the
 * stale pointers in the tree, then returns the root AST node to
 * start the program.
 */
ctr_tnode* ctr_serializer_unserialize(char* filename) {
	FILE *f;
	uint64_t j=0;
	size_t s = 0;
	uintptr_t p; /* addressbook entry */
	uintptr_t p2; /* corrected address entry */
	uintptr_t ob; /* old base */
	uint64_t cnt; /* counter */
	uintptr_t tp; /* the new pointer (replacement) */
	uintptr_t otp; /* the old pointer (to be replaced) */
	uintptr_t pe; /* pe */
	uintptr_t sz;
	s = fsize(filename);
	np = malloc(sizeof(char)*s);
	if (!np) { printf("no memory.\n"); exit(1);}
	f = fopen(filename,"rb");
	fread(np, sizeof(char), s, f);
	fclose(f);
	ctr_default_header = (ctr_ast_header*) (np);
	sz = ctr_default_header->size_of_address_book;
	ob = ctr_default_header->start_block;
	pe = ctr_default_header->program_entry_point;
	cnt = ctr_default_header->num_of_swizzles;
	ctr_malloc_swizzle_adressbook = (uintptr_t*) (np + sizeof(ctr_ast_header));
	pe = pe - (uintptr_t) ob;
	pe = pe + (uintptr_t) np;
	/* perform pointer swizzling to restore the tree from the image */
	for(j = 0; j<cnt; j++) {
		p = (uintptr_t) *(ctr_malloc_swizzle_adressbook); /* p is an old address */
		p2 = p - (uintptr_t) ob + (uintptr_t) np; /* subtract the base from p and add the new base */
		otp = *((uintptr_t* )p2); /* retrieve the old pointer from p2 */
		if (otp != 0) {
			tp = otp - (uintptr_t) ob + (uintptr_t) np; /* correct the pointer for the new base address */
			*((uintptr_t* )p2) = tp; /* replace the old pointer with the new one */
		}
		ctr_malloc_swizzle_adressbook += 1; /* take an address from the book */
	}
	return (ctr_tnode*) pe;
}


/**
 * DebugTree
 *
 * For debugging purposes, prints the internal AST.
 */
void ctr_internal_debug_tree(ctr_tnode* ti, int indent) {
	char* str;
	char* vbuf;
	ctr_tlistitem* li;
	ctr_tnode* t;
	if (indent>20) exit(1); 
	li = ti->nodes;
	t = li->node;
	while(1) {
		int i;
		for (i=0; i<indent; i++) printf(" ");
		str = calloc(40, sizeof(char));
		switch (t->type) {
			case CTR_AST_NODE_EXPRASSIGNMENT:
				str = "ASSIGN\0";
				break;
			case CTR_AST_NODE_EXPRMESSAGE:
				str = "MESSAG\0";
				break;
			case CTR_AST_NODE_UNAMESSAGE:
				str = "UMSSAG\0";
				break;
			case CTR_AST_NODE_KWMESSAGE:
				str = "KMSSAG\0";
				break;
			case CTR_AST_NODE_BINMESSAGE:
				str = "BMSSAG\0";
				break;
			case CTR_AST_NODE_LTRSTRING:
				str = "STRING\0";
				break;
			case CTR_AST_NODE_REFERENCE:
				str = "REFRNC\0";
				break;
			case CTR_AST_NODE_LTRNUM:
				str = "NUMBER\0";
				break;
			case CTR_AST_NODE_CODEBLOCK:
				str = "CODEBL\0";
				break;
			case CTR_AST_NODE_RETURNFROMBLOCK:
				str = "RETURN\0";
				break;
			case CTR_AST_NODE_PARAMLIST:
				str = "PARAMS\0";
				break;
			case CTR_AST_NODE_INSTRLIST:
				str = "INSTRS\0";
				break;
			case CTR_AST_NODE_ENDOFPROGRAM:
				str = "EOPROG\0";
				break;
			case CTR_AST_NODE_NESTED:
				str = "NESTED\0";
				break;
			case CTR_AST_NODE_LTRBOOLFALSE:
				str = "BFALSE\0";
				break;
			case CTR_AST_NODE_LTRBOOLTRUE:
				str = "BLTRUE\0";
				break;
			case CTR_AST_NODE_LTRNIL:
				str = "LTRNIL\0";
				break;
			default:
				str = "UNKNW?\0";
				break;
		}
		vbuf = calloc(sizeof(char),t->vlen+1);
		strncpy(vbuf, t->value, t->vlen);
		printf("%s %s (%p)\n", str, vbuf, (void*) t);
		if (t->nodes) ctr_internal_debug_tree(t, indent + 1);
		if (!li->next) break; 
		li = li->next;
		t = li->node;
		
	}
}

/**
 * @internal
 * Internal plugin loader.
 * Tries to locate a plugin by its name.
 *
 * In Citrine a plugin is loaded automatically as soon as a symbol
 * has been enountered that cannot be found in global scope.
 *
 * Before raising an error Citrine will attempt to load a plugin
 * to meet the dependency.
 *
 * To install a plugin, copy the plugin folder to the mods folder
 * in the working directory of your script.
 *
 * So, for instance, to install the 'Coffee Percolator Plugin',
 * we copy the libctrpercolator.so in folder mods/percolator/,
 * the resulting path is: mods/percolator/libctrpercolator.so.
 *
 * General path format:
 *
 * mods/X/libctrX.so
 *
 * where X is the name of the object the plugin offers in lowercase.
 *
 * On loading, the plugin will get a chance to add its objects to the world
 * through a constructor function.
 */
void* ctr_internal_plugin_find(ctr_object* key) {
	ctr_object* modNameObject = ctr_internal_cast2string(key);
	void* handle;
	char  pathName[1024];
	char  pathNameMod[1024];
	char* modName;
	char* modNameLow;
	CTR_2CSTR(modName, modNameObject);
	modNameLow = modName;
	for ( ; *modNameLow; ++modNameLow) *modNameLow = tolower(*modNameLow);
	if (getcwd(pathName, sizeof(pathName)) == NULL) return NULL;
	snprintf(pathNameMod, 1024,"%s/mods/%s/libctr%s.so", pathName, modName, modName);
	if (access(pathNameMod, F_OK) == -1) return NULL;
	handle =  dlopen(pathNameMod, RTLD_NOW);
	return handle;
}
