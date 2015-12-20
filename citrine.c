
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
	if (!f) { printf("Unable to open file!"); exit(1); }
	fwrite(chunk, sizeof(char), 8000, f);
	fclose(f);
}

ctr_tnode* ctr_serializer_unserialize() {
	
	FILE *f;
	int j=0;
	void* ptr;
	long s = fsize("dump.ast");
	
	
	
	uintptr_t p; /* addressbook entry */
	uintptr_t p2; /* corrected address entry */
	char* np; /* new memory */
	uintptr_t ob; /* old base */
	uint64_t cnt; /* counter */
	uintptr_t tp;
	uintptr_t otp;
	
	printf("size %lu \n", s);
	
	
	np = malloc(sizeof(char)*s);
	
	if (!np) { printf("no memory.\n"); exit(1);}
	
	printf("NP = %p \n", np);
	
		
	
	printf("Loading AST dump file from disk.\n");
	
	f = fopen("dump.ast","rb");
	
	
	
	fread(np, sizeof(char), s, f);
	
	printf("NP = %p \n", np);
	
	fclose(f);
	abook = (uintptr_t*) np;
	np += (uintptr_t) floor( s /4 ); /* reserve 25% of space for address book */
	printf("New memory starts at: %p \n",  np);
	
	cnt = (uint64_t) *(abook);
	
	printf("test uint64_t : %" PRIu64 "\n", cnt);
	
	abook += sizeof(uint64_t);
	ob = (uintptr_t) *(abook);
	
	printf("Base address is: %p \n", ob);
	
	
	for(j = 0; j<cnt; j++) {
		abook += sizeof(uintptr_t);
		p = *(abook);
		p2 = p - (uintptr_t) ob + (uintptr_t) np;
		printf("p = %p, p2 = %p \n", p, p2);
		otp = *((uintptr_t* )p2);
		printf("otp = %p \n",otp);
		tp = otp - (uintptr_t) ob + (uintptr_t) np;
		printf("tp = %p \n", tp);
		*((uintptr_t* )p2) = tp;
		
		
		
	}

	return (ctr_tnode*) np;
}

int ctr_mode_debug = 0;
ctr_object* error;
int main(int argc, char* argv[]) {
	char* prg;
	ctr_tnode* program;
	ctr_argc = argc;
	ctr_argv = argv;
	error = NULL;
	chunk_ptr = 0;
	
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
	prg = ctr_internal_readf(argv[1]);
	
	
	/*save*/
	program = ctr_dparse_parse(prg);
	ctr_serializer_serialize(program);
	
	/*load*/
	program = ctr_serializer_unserialize();
	ctr_initialize_world();
	ctr_cwlk_run(program);
	
	
	
	/*just run*/
	/*ctr_initialize_world();
	ctr_cwlk_run(program);*/
	exit(0);
}
