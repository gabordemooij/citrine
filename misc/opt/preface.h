static char* embedded_program = "\
Media _datastart.\
";

int main(int argc, char *argv[]) {
	uint64_t program_text_size = 0;
	ctr_tnode* program;
	ctr_init();
	program_text_size = strlen(embedded_program);
	ctr_program_length = program_text_size;
	program = ctr_cparse_parse(embedded_program, "/embedded.ctr");
	ctr_initialize_world();
	ctr_cwlk_run(program);
	ctr_gc_sweep(1);
	ctr_heap_free_rest();
	return 0;
}