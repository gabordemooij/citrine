
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>

#include "citrine.h"

int ctr_mode_debug = 0;
ctr_object* error;
int main(int argc, char* argv[]) {
	char* prg;
	ctr_tnode* program;
	ctr_argc = argc;
	ctr_argv = argv;
	error = NULL;
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
	program = ctr_dparse_parse(prg);
	ctr_initialize_world();
	ctr_cwlk_run(program);
	exit(0);
}
