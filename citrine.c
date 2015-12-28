
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>


#include "citrine.h"

char* raw;
char* np; /* new memory */
int fsize(char* x) {
  int size;
  FILE* fh;
  fh = fopen("dump.ast", "rb");
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

void ctr_serializer_serialize(ctr_tnode* t) {
	FILE *f;
	f = fopen("dump.ast","wb");
	abook = (uintptr_t*) chunk;
	if (!f) { printf("Unable to open file!"); exit(1); }
	fwrite(chunk, sizeof(char), measure_code+measure, f);
	fclose(f);
}

ctr_tnode* ctr_serializer_unserialize() {
	FILE *f;
	int j=0;
	void* ptr;
	long s = fsize("dump.ast");
	uintptr_t p; /* addressbook entry */
	uintptr_t p2; /* corrected address entry */
	uintptr_t ob; /* old base */
	uint64_t cnt; /* counter */
	uintptr_t tp; /* the new pointer (replacement) */
	uintptr_t otp; /* the old pointer (to be replaced) */
	uintptr_t pe; /* pe */
	uintptr_t sz;
	np = calloc(sizeof(char),s);
	if (!np) { printf("no memory.\n"); exit(1);}
	f = fopen("dump.ast","rb");
	fread(np, sizeof(char), s, f);
	raw = np;
	fclose(f);
	abook = (uintptr_t*) np; /* set new pointer to loaded image */
	cnt = (uint64_t) *(abook); /* first entry in address book is a 64bit number indicating the number of swizzles */
	abook += 1;/*sizeof(uint64_t);(*/
	sz = (uintptr_t) *(abook);
	abook += 1;/*sizeof(uint64_t); /* move to next entry in addressbook */
	ob = (uintptr_t) *(abook); /* second entry in address book is the old base address to use for swizzling */
	abook += 1;
	pe = (uintptr_t) *(abook); /* program entry point */
	pe = pe - (uintptr_t) ob;
	pe = pe + (uintptr_t) np;
	/* perform pointer swizzling to restore the tree from the image */
	for(j = 0; j<cnt; j++) {
		abook += 1; /*sizeof(uintptr_t); /* take an address from the book */
		p = (uintptr_t) *(abook); /* p is an old address */
		p2 = p - (uintptr_t) ob + (uintptr_t) np; /* subtract the base from p and add the new base */
		otp = *((uintptr_t* )p2); /* retrieve the old pointer from p2 */
		if (otp == 0) { continue; }
		tp = otp - (uintptr_t) ob + (uintptr_t) np; /* correct the pointer for the new base address */
		*((uintptr_t* )p2) = tp; /* replace the old pointer with the new one */
	}
	return (ctr_tnode*) pe;
}

int ctr_mode_debug = 0;
ctr_object* error;

int main(int argc, char* argv[]) {
	char* prg;
	int k;
	ctr_tnode* program;
	ctr_argc = argc;
	ctr_argv = argv;
	error = NULL;
	chunk_ptr = 0;
	ctr_mode_compile = 0;
	ctr_mode_roundtrip = 0;
	ctr_mode_load = 0;
	if (argc < 2) {
		printf("\nWelcome to Citrine v0.2\n");
		printf("Written by Gabor de Mooij (c) copyright 2015\n");
		printf("The Citrine Programming Language BSD license.\n------------------------\n\n");
		printf("Usage: ctr mycode.ctr \n");
		printf("Debugger: ctr mycode.ctr --debug\n\n");
		exit(1);
	}
	if (argc == 3) if (strcmp(argv[2],"--debug")==0) ctr_mode_debug = 1;
	if (ctr_mode_debug == 1) printf("Debugger is ON.\n");
	if (argc == 3) if (strcmp(argv[2],"--compile")==0) {
		ctr_mode_compile = 1;
		ctr_argc--;
	}
	if (argc == 3) if (strcmp(argv[2],"--load")==0) {
		ctr_mode_load = 1;
		ctr_argc--;
	}
	if (argc == 3) if (strcmp(argv[2],"--roundtrip")==0) {
		ctr_mode_roundtrip = 1;
		ctr_argc--;
	}
	if (ctr_mode_roundtrip) {
		prg = ctr_internal_readf(argv[1]);
		xallocmode = 0;
		measure = 0;
		program = ctr_dparse_parse(prg);
		program = NULL;
		xallocmode = 1;
		chunk_ptr = 0;
		chunk = 0;
		program = ctr_dparse_parse(prg);
		ctr_serializer_serialize(program);
		free(chunk);
		abook = 0;
		chunk_ptr = 0;
		program = NULL;
		xallocmode = 0;
		program = ctr_serializer_unserialize();
		ctr_initialize_world();
		ctr_cwlk_run(program);
		exit(0);
	} else {
		prg = ctr_internal_readf(argv[1]);
		xallocmode = 0;
		program = ctr_dparse_parse(prg);
		ctr_initialize_world();
		ctr_cwlk_run(program);
		exit(0);
	}
}
