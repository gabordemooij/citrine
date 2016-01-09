
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
	memcpy( ctr_malloc_chunk, ctr_default_header, sizeof(ctr_ast_header));/* append to addressbook, new header */
	if (!f) { printf("Unable to open file!"); exit(1); }
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
	printf("Display info about AST      : ctr -i file \n");
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
	int option_symbol; 
	while ((option_symbol = getopt(argc, argv, "c:ri")) != -1) {
		switch (option_symbol) { 
			case 'c':
				ctr_mode_compile = 1;
				ctr_mode_compile_save_as = calloc(sizeof(char), 255);
				strncpy(ctr_mode_compile_save_as, optarg, 254);
				break; 
			case 'r':
				ctr_mode_load = 1;
				break;
			case 'i':
				ctr_mode_info = 1;
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

/**
 * Citrine Application Main Start
 * Bootstraps the Citrine Application.
 */
int main(int argc, char* argv[]) {
	char* prg;
	ctr_tnode* program;
	ctr_argc = argc;
	ctr_argv = argv;
	ctr_malloc_chunk_pointer = 0;
	ctr_mode_compile = 0;
	ctr_mode_load = 0;
	ctr_cli_read_args(argc, argv);
	if (ctr_mode_compile) {
		prg = ctr_internal_readf(ctr_mode_input_file);
		ctr_malloc_mode = 0;
		ctr_malloc_measured_size_addressbook = sizeof(ctr_ast_header);/* adds up to normal addressbook size */
		program = ctr_dparse_parse(prg);
		program = NULL;
		ctr_malloc_mode = 1;
		ctr_malloc_chunk_pointer = 0;
		ctr_malloc_chunk = 0;
		program = ctr_dparse_parse(prg);
		ctr_serializer_serialize(program);
		free(ctr_malloc_chunk);
		free(prg);
		exit(0);
	}
	else if (ctr_mode_load) {
		ctr_malloc_mode = 0;
		ctr_malloc_chunk_pointer = 0;
		ctr_malloc_chunk = 0;
		ctr_malloc_swizzle_adressbook = 0;
		program = NULL;
		program = ctr_serializer_unserialize(ctr_mode_input_file);
		ctr_initialize_world();
		ctr_cwlk_run(program);
		exit(0);
	}
	else if (ctr_mode_info) {
		ctr_malloc_mode = 0;
		ctr_malloc_chunk_pointer = 0;
		ctr_malloc_chunk = 0;
		ctr_malloc_swizzle_adressbook = 0;
		ctr_serializer_info(ctr_mode_input_file);
		exit(0);
	}
	else {
		prg = ctr_internal_readf(ctr_mode_input_file);
		ctr_malloc_mode = 0;
		program = ctr_dparse_parse(prg);
		ctr_initialize_world();
		ctr_cwlk_run(program);
		free(program);
		exit(0);
	}
	return 0;
}
