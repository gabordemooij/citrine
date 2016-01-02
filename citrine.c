
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include <unistd.h>
#include "citrine.h"

char* raw;
char* np; /* new memory */
int fsize(char* x) {
  int size;
  FILE* fh;
  fh = fopen(ctr_mode_input_file, "rb");
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
	f = fopen(ctr_mode_compile_save_as,"wb");
	abook = (uintptr_t*) chunk;
	memcpy( chunk, &teller, sizeof teller );
	if (!f) { printf("Unable to open file!"); exit(1); }
	fwrite(chunk, sizeof(char), measure_code+measure, f);
	fclose(f);
}

ctr_tnode* ctr_serializer_unserialize() {
	FILE *f;
	uint64_t j=0;
	void* ptr;
	size_t s = 0;
	uintptr_t p; /* addressbook entry */
	uintptr_t p2; /* corrected address entry */
	uintptr_t ob; /* old base */
	uint64_t cnt; /* counter */
	uintptr_t tp; /* the new pointer (replacement) */
	uintptr_t otp; /* the old pointer (to be replaced) */
	uintptr_t pe; /* pe */
	uintptr_t sz;
	int t = 0;
	s = fsize(ctr_mode_input_file);
	np = calloc(sizeof(char),s);
	if (!np) { printf("no memory.\n"); exit(1);}
	f = fopen(ctr_mode_input_file,"rb");
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
		if (otp != 0) {
			tp = otp - (uintptr_t) ob + (uintptr_t) np; /* correct the pointer for the new base address */
			*((uintptr_t* )p2) = tp; /* replace the old pointer with the new one */
		}
	
	}
	return (ctr_tnode*) pe;
}

int ctr_mode_debug = 0;
ctr_object* error;


/**
 * CommandLine Display Welcome Message
 * Displays a Welcome message, copyright information,
 * version information and usage.
 */
void ctr_cli_welcome() {
	printf("\n");
	printf("Citrine Programming Language V 0.3.\n");
	printf("\n");
	printf("--------------------------------------------------\n");
	printf("\n");
	printf("Written by: Gabor de Mooij (c) copyright 2016, Licensed BSD.\n");
	printf("Quick Usage Examples:\n");
	printf("Run a CTR file (interpreter): ctr file \n");
	printf("Compile to binary AST       : ctr -c outputfile file \n");
	printf("Run an AST                  : ctr -r file \n");
	printf("\n");
	printf("--------------------------------------------------\n");
	printf("\n");
	printf("For more information enter  : man ctr \n");
	printf("Or visit the website: http://citrine-lang.org.\n");
	printf("\n");
}



/**
 * CommandLine Read Arguments
 * Parses command line arguments and sets global settings accordingly.
 */
void ctr_cli_read_args(int argc, char* argv[]) {
	int bflag, option_symbol, fd; 
	bflag = 0; 
	while ((option_symbol = getopt(argc, argv, "c:r")) != -1) { 
		switch (option_symbol) { 
			case 'c':
				ctr_mode_compile = 1;
				ctr_mode_compile_save_as = calloc(sizeof(char), 255);
				strncpy(ctr_mode_compile_save_as, optarg, 254);
				break; 
			case 'r':
				ctr_mode_load = 1;
				break;
			default: 
				ctr_cli_welcome();
				exit(0);
				break;
		} 
	}
	
	if (argc == 1) {
		ctr_cli_welcome();
		exit(0);
	}
	
	ctr_mode_input_file = (char*) calloc(sizeof(char), 255);
	strncpy(ctr_mode_input_file, argv[optind], 254);
}

int main(int argc, char* argv[]) {
	char* prg;
	int k;
	ctr_tnode* program;
	ctr_argc = argc;
	ctr_argv = argv;
	error = NULL;
	chunk_ptr = 0;
	ctr_mode_compile = 0;
	ctr_mode_load = 0;
	ctr_cli_read_args(argc, argv);
	if (ctr_mode_compile) {
		prg = ctr_internal_readf(ctr_mode_input_file);
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
		free(prg);
		exit(0);
	}
	else if (ctr_mode_load) {
		xallocmode = 0;
		chunk_ptr = 0;
		chunk = 0;
		abook = 0;
		program = NULL;
		program = ctr_serializer_unserialize();
		ctr_initialize_world();
		ctr_cwlk_run(program);
		exit(0);
	}
	else {
		prg = ctr_internal_readf(ctr_mode_input_file);
		xallocmode = 0;
		program = ctr_dparse_parse(prg);
		ctr_initialize_world();
		ctr_cwlk_run(program);
		free(program);
		exit(0);
	}
	return 0;
}
