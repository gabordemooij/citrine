#include <inttypes.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include "citrine.h"
#include "siphash.h"

#ifdef __MINGW32__
#include <winsock2.h>
#endif

int ctr_argc;
char** ctr_argv;

/**
 * CommandLine Display Welcome Message
 * Displays a Welcome message, copyright information,
 * version information and usage.
 */
void ctr_cli_welcome() {
	printf("\n");
	#ifdef langNL
		printf("Citrine Programmeertaal V 0.7.8\n");
		printf("Geschreven door Gabor de Mooij © alle rechten voorbehouden 2018, Licensie BSD.\n");
	#else
		printf("Citrine Programming Language V 0.7.8\n");
		printf("Written by Gabor de Mooij © copyright 2018, Licensed BSD.\n");
	#endif
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
	ctr_gc_mode = 1; /* default GC mode: activate GC */
	ctr_argc = argc;
	ctr_argv = argv;
	ctr_gc_memlimit = 8388608;
	ctr_callstack_index = 0;
	ctr_source_map_head = NULL;
	ctr_source_mapping = 0;
	CtrStdFlow = NULL;
	ctr_program_security_profile = 0;
	ctr_program_tick = 0;
	ctr_cli_read_args(argc, argv);
	#ifdef __MINGW32__
   	WORD versionWanted = MAKEWORD(2, 2);
   	WSADATA wsaData;
   	WSAStartup(versionWanted, &wsaData);
	#endif
	ctr_source_mapping = 1;
	ctr_clex_keyword_me = CTR_DICT_ME;
	ctr_clex_keyword_my = CTR_DICT_MY;
	ctr_clex_keyword_var = CTR_DICT_VAR;
	ctr_clex_keyword_my_len = strlen( ctr_clex_keyword_my );
	ctr_clex_keyword_var_len = strlen( ctr_clex_keyword_var );
	ctr_clex_keyword_me_icon = CTR_DICT_ME_ICON;
	ctr_clex_keyword_my_icon = CTR_DICT_MY_ICON;
	ctr_clex_keyword_var_icon = CTR_DICT_VAR_ICON;
	ctr_clex_keyword_my_icon_len = strlen( ctr_clex_keyword_my_icon );
	ctr_clex_keyword_var_icon_len = strlen( ctr_clex_keyword_var_icon );
	ctr_clex_string_interpolation_start_len = strlen( CTR_DICT_STR_IPOL_START );
	ctr_clex_string_interpolation_stop_len = strlen( CTR_DICT_STR_IPOL_STOP );
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
