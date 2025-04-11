#include "citrine.h"

#ifdef EMBED
	#include "misc/opt/embed.h"
#endif

int ctr_argc;
char** ctr_argv;

char* ctr_mode_input_file;
char* ctr_mode_dict_file;
char* ctr_mode_hfile1;
char* ctr_mode_hfile2;
ctr_size ctr_clex_keyword_eol_len;
ctr_size ctr_clex_keyword_num_sep_dec_len;
ctr_size ctr_clex_keyword_num_sep_tho_len;
ctr_size ctr_clex_keyword_assignment_len;
ctr_size ctr_clex_keyword_return_len;
ctr_size ctr_clex_keyword_chain_len;

/**
 * CommandLine Display Welcome Message
 * Displays a Welcome message, copyright information,
 * version information and usage.
 */
void ctr_cli_welcome() {
	printf( CTR_MSG_WELCOME );
	printf( CTR_MSG_COPYRIGHT );
	printf( CTR_VERSION );
	printf("\n");
}

void* ctr_data_blob(unsigned int* len) {
	#ifdef EMBED
	*len = CtrBlob_len;
	return CtrBlob;
	#else
	*len = 0;
	return NULL;
	#endif
}

/**
 * CommandLine Read Arguments
 * Parses command line arguments and sets global settings accordingly.
 */
void ctr_cli_read_args(int argc, char* argv[]) {
	if (argc < 2) {
		ctr_cli_welcome();
		exit(0);
	} else {
		ctr_mode_input_file = (char*) ctr_heap_allocate_tracked( sizeof( char ) * 255 );
		strncpy(ctr_mode_input_file, argv[1], 254);
	}
}

/**
 * Inits the Citrine environment.
 */
int ctr_init() {
	ctr_in_message = 0;
	ctr_callstack_index = 0;
	ctr_source_map_head = NULL;
	ctr_source_mapping = 0;
	CtrStdFlow = NULL;
	ctr_source_mapping = 1;
	ctr_deserialize_mode = 0;
	ctr_clex_keyword_me_icon = CTR_DICT_ME_ICON;
	ctr_clex_keyword_my_icon = CTR_DICT_MY_ICON;
	ctr_clex_keyword_var_icon = CTR_DICT_VAR_ICON;
	ctr_clex_keyword_my_icon_len = strlen( ctr_clex_keyword_my_icon );
	ctr_clex_keyword_var_icon_len = strlen( ctr_clex_keyword_var_icon );
	ctr_clex_keyword_eol_len = strlen( CTR_DICT_END_OF_LINE );
	ctr_clex_keyword_chain_len = strlen( CTR_DICT_MESSAGE_CHAIN );
	ctr_clex_keyword_num_sep_dec_len = strlen( CTR_DICT_NUM_DEC_SEP );
	ctr_clex_keyword_num_sep_tho_len = strlen( CTR_DICT_NUM_THO_SEP );
	ctr_clex_keyword_qo_len = strlen( CTR_DICT_QUOT_OPEN );
	ctr_clex_keyword_qc_len = strlen( CTR_DICT_QUOT_CLOSE );
	ctr_clex_keyword_assignment_len = strlen( CTR_DICT_ASSIGN );
	ctr_clex_keyword_return_len = strlen( CTR_DICT_RETURN );
	ctr_clex_param_prefix_char = CTR_DICT_PARAMETER_PREFIX[0];
	ctr_gc_memlimit = 64 * 1000000; /* Default memory limit: 64MB */
	ctr_gc_mode = 1;                /* Default GC mode: regular GC. */
	return 0;
}

/**
 * Citrine Application Main Start
 * Bootstraps the Citrine Application.
 *
 */
#ifndef EMBED
int main(int argc, char* argv[]) {
	char* prg;
	ctr_tnode* program;
	uint64_t program_text_size = 0;
	ctr_argc = argc;
	ctr_argv = argv;
	ctr_init();
	//Command line options
	ctr_cli_read_args(argc, argv);
	prg = ctr_internal_readf(ctr_mode_input_file, &program_text_size);
	//Advanced parameters - environment
	char* env_param_citrine_memory_limit_mb   = getenv("CITRINE_MEMORY_LIMIT_MB");   // - memory limit in MB
	char* env_param_citrine_memory_mode       = getenv("CITRINE_MEMORY_MODE");       // - GC mode
	if (env_param_citrine_memory_limit_mb)   ctr_gc_memlimit = atoi(env_param_citrine_memory_limit_mb) * 1000000;
	if (env_param_citrine_memory_mode)       ctr_gc_mode = atoi(env_param_citrine_memory_mode);
	program = ctr_cparse_parse(prg, ctr_mode_input_file);
	if (program == NULL) {
		fwrite(CtrStdFlow->value.svalue->value, CtrStdFlow->value.svalue->vlen, 1, stderr);
		exit(1);
	}
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
	if (CtrStdFlow && CtrStdFlow != CtrStdExit) {
		exit(1);
	}
	exit(0);
	return 0;
}
#endif