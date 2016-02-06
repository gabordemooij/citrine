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
#include "siphash.h"

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
                /* Prevent segfaults with wrong number of arguments  */
                if ( argc < 4 ) {
                    printf("Too few arguments!\n");
                    ctr_cli_welcome();
                    exit(1);
                }
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
 *
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
		/*ctr_internal_debug_tree(program,1); -- for debugging */
		ctr_initialize_world();
		ctr_cwlk_run(program);
		free(program);
		exit(0);
	}
	return 0;
}
