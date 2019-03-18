#include <inttypes.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include <unistd.h>
#include "citrine.h"
#include "siphash.h"

int ctr_argc;
char** ctr_argv;

char* ctr_mode_input_file;
char* ctr_mode_dict_file;
char* ctr_mode_hfile1;
char* ctr_mode_hfile2;
char ctr_flag_sandbox;
uint16_t ctr_sandbox_steps = 0;

/**
 * CommandLine Display Welcome Message
 * Displays a Welcome message, copyright information,
 * version information and usage.
 */
void ctr_cli_welcome() {
	printf("\n");
	printf( CTR_MSG_WELCOME );
	printf( CTR_MSG_COPYRIGHT );
	printf( CTR_VERSION );
	printf("\n");
}

/**
 * CommandLine Read Arguments
 * Parses command line arguments and sets global settings accordingly.
 */
int ctr_cli_read_args(int argc, char* argv[]) {
	int mode = 0;
	if (argc == 1) {
		ctr_cli_welcome();
		exit(0);
	}
	if (strncmp(argv[1],"-g", 2)==0) {
		if (argc < 4) {
			ctr_print_error( CTR_MSG_USAGE_G, 1 );
		}
		ctr_mode_hfile1 = (char*) ctr_heap_allocate_tracked( sizeof( char ) * 255 );
		strncpy(ctr_mode_hfile1, argv[2], 254);
		ctr_mode_hfile2 = (char*) ctr_heap_allocate_tracked( sizeof( char ) * 255 );
		strncpy(ctr_mode_hfile2, argv[3], 254);
		mode = 2;
	}
	if (strncmp(argv[1],"-t", 2)==0) {
		if (argc < 4) {
			ctr_print_error( CTR_MSG_USAGE_T, 1 );
		}
		ctr_mode_dict_file = (char*) ctr_heap_allocate_tracked( sizeof( char ) * 255 );
		strncpy(ctr_mode_dict_file, argv[2], 254);
		ctr_mode_input_file = (char*) ctr_heap_allocate_tracked( sizeof( char ) * 255 );
		strncpy(ctr_mode_input_file, argv[3], 254);
		mode = 1;
	} else {
		if (strncmp(argv[1],"-s", 2)==0) {
			ctr_flag_sandbox = 1;
			ctr_mode_input_file = (char*) ctr_heap_allocate_tracked( sizeof( char ) * 255 );
			strncpy(ctr_mode_input_file, argv[2], 254);
		} else {
			ctr_mode_input_file = (char*) ctr_heap_allocate_tracked( sizeof( char ) * 255 );
			strncpy(ctr_mode_input_file, argv[1], 254);
		}
	}
	return mode;
}

/**
 * Citrine Application Main Start
 * Bootstraps the Citrine Application.
 *
 */
int main(int argc, char* argv[]) {
	char* prg;
	ctr_tnode* program;
	uint64_t program_text_size = 0;
	ctr_gc_mode = 9; /* default GC mode: activate GC 1 + Pool 8 = 9 */
	ctr_argc = argc;
	ctr_argv = argv;
	ctr_in_message = 0;
	ctr_gc_memlimit = 8500000;
	ctr_callstack_index = 0;
	ctr_sandbox_steps = 0;
	ctr_source_map_head = NULL;
	ctr_source_mapping = 0;
	CtrStdFlow = NULL;
	ctr_source_mapping = 1;
	ctr_clex_keyword_me_icon = CTR_DICT_ME_ICON;
	ctr_clex_keyword_my_icon = CTR_DICT_MY_ICON;
	ctr_clex_keyword_var_icon = CTR_DICT_VAR_ICON;
	ctr_clex_keyword_my_icon_len = strlen( ctr_clex_keyword_my_icon );
	ctr_clex_keyword_var_icon_len = strlen( ctr_clex_keyword_var_icon );
	int mode = ctr_cli_read_args(argc, argv);
	if (mode == 1) {
		prg = ctr_internal_readf(ctr_mode_input_file, &program_text_size);
		ctr_translate_program(prg, ctr_mode_input_file);
		exit(0);
	}
	if (mode == 2) {
		ctr_translate_generate_dicts(ctr_mode_hfile1, ctr_mode_hfile2);
		exit(0);
	}
	prg = ctr_internal_readf(ctr_mode_input_file, &program_text_size);
	program = ctr_cparse_parse(prg, ctr_mode_input_file);
	/*ctr_internal_debug_tree(program,1); -- for debugging */
	ctr_initialize_world();
	ctr_cwlk_run(program);
	ctr_gc_sweep(1);
	ctr_heap_free( prg );
	ctr_heap_free_rest();
	//For memory profiling
	if ( ctr_gc_alloc != 0 ) {
		fprintf( stderr, "[WARNING] Citrine has detected an internal memory leak of: %" PRIu64 " bytes.\n", ctr_gc_alloc );
		exit(1);
	}
	exit(0);
	return 0;
}
