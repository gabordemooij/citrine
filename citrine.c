#include <inttypes.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
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
	printf("Citrine Programming Language V 0.6.\n");
	printf("\n");
	printf("--------------------------------------------------\n");
	printf("\n");
	printf("Written by: Gabor de Mooij (c) copyright 2016, Licensed BSD.\n");
	printf("Quick Usage Examples:\n");
	printf("Run a CTR file: ctr file \n");
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
	if (argc == 1) {
		ctr_cli_welcome();
		exit(0);
	}
	ctr_mode_input_file = (char*) ctr_heap_allocate_tracked( sizeof( char ) * 255 );
	strncpy(ctr_mode_input_file, argv[1], 254);
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
	ctr_gc_mode = 0; /* default GC mode: activate GC and recycle used objects */
	ctr_argc = argc;
	ctr_argv = argv;
	ctr_gc_memlimit = 8388608;
	ctr_callstack_index = 0;
	ctr_source_map_head = NULL;
	ctr_source_mapping = 0;
	CtrStdFlow = NULL;
	ctr_cli_read_args(argc, argv);
	ctr_source_mapping = 1;
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
		printf( "[WARNING] Citrine has detected an internal memory leak of: %" PRIu64 " bytes.\n", ctr_gc_alloc );
		exit(1);
	}
	exit(0);
	return 0;
}
