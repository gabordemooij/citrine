
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>

#include "citrine.h"

int debug = 0;
ctr_object* error;
int main(int argc, char* argv[]) {
	__argc = argc;
	__argv = argv;
	char* prg;
	error = NULL;
	if (argc < 2) {
		printf("\nWelcome to Citrine v0.1\n");
		printf("Written by Gabor de Mooij (c) copyright 2014\n");
		printf("The Citrine Programming Language BSD license.\n------------------------\n\n");
		printf("Usage: ctr mycode.ctr \n");
		printf("Debugger: ctr mycode.ctr --debug\n\n");
		exit(1);
	}
	if (argc == 3) if (strcmp(argv[2],"--debug")==0) debug = 1;
	if (debug == 1) printf("Debugger is ON.\n");
	prg = readf(argv[1]);
	tnode* program = dparse_parse(prg);
	ctr_initialize_world();
	cwlk_run(program);
	exit(0);
}
