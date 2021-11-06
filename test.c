#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#include "citrine.h"
#include "siphash.h"


int ctr_internal_testcounter = 1;

/**
 * Displays test no, or aborts testing because of failed assumption.
 * 
 * @param integer t
 */
void ctr_test( int t ) {
	if (t) {
		printf("[%d]", ctr_internal_testcounter++);
	} else {
		printf("Error test #%d failed.", ctr_internal_testcounter);
		exit(1);
	}
}

/**
 * Perform token test
 */
void ctr_coretest_tokens() {
	char* buffer;
	int token;
	ctr_program_length = 40;
	buffer = calloc(40,1);
	sprintf( buffer, "%s", CTR_DICT_PAREN_OPEN );
	ctr_clex_load( buffer );
	token = ctr_clex_tok();
	ctr_test(token == CTR_TOKEN_PAROPEN);
	ctr_test(strlen(ctr_clex_tok_value())==0);
	ctr_test(strcmp(ctr_clex_tok_describe(token),CTR_DICT_PAREN_OPEN)==0);
	free(buffer);
	buffer = calloc(40,1);
	sprintf( buffer, "%s", CTR_DICT_PAREN_CLOSE );
	ctr_clex_load( buffer );
	token = ctr_clex_tok();
	ctr_test(token == CTR_TOKEN_PARCLOSE);
	ctr_test(strlen(ctr_clex_tok_value())==0);
	ctr_test(strcmp(ctr_clex_tok_describe(token),CTR_DICT_PAREN_CLOSE)==0);
	free(buffer);
	buffer = calloc(40,1);
	sprintf( buffer, "%s", CTR_DICT_BLOCK_START );
	ctr_clex_load( buffer );
	token = ctr_clex_tok();
	ctr_test(token == CTR_TOKEN_BLOCKOPEN);
	ctr_test(strlen(ctr_clex_tok_value())==0);
	ctr_test(strcmp(ctr_clex_tok_describe(token),CTR_DICT_BLOCK_START)==0);
	free(buffer);
	buffer = calloc(40,1);
	sprintf( buffer, "%s", CTR_DICT_BLOCK_END );
	ctr_clex_load( buffer );
	token = ctr_clex_tok();
	ctr_test(token == CTR_TOKEN_BLOCKCLOSE);
	ctr_test(strlen(ctr_clex_tok_value())==0);
	ctr_test(strcmp(ctr_clex_tok_describe(token),CTR_DICT_BLOCK_END)==0);
	free(buffer);
	buffer = calloc(40,1);
	sprintf( buffer, "%s", CTR_DICT_END_OF_LINE );
	ctr_clex_load( buffer );
	token = ctr_clex_tok();
	ctr_test(token == CTR_TOKEN_DOT);
	ctr_test(strlen(ctr_clex_tok_value())==0);
	ctr_test(strcmp(ctr_clex_tok_describe(token),CTR_DICT_END_OF_LINE)==0);
	free(buffer);
	buffer = calloc(40,1);
	sprintf( buffer, "%s", CTR_DICT_MESSAGE_CHAIN );
	ctr_clex_load( buffer );
	token = ctr_clex_tok();
	ctr_test(token == CTR_TOKEN_CHAIN);
	ctr_test(strlen(ctr_clex_tok_value())==0);
	ctr_test(strcmp(ctr_clex_tok_describe(token),CTR_DICT_MESSAGE_CHAIN)==0);
	free(buffer);
	buffer = calloc(40,1);
	sprintf( buffer, "%s", CTR_DICT_PARAMETER_PREFIX );
	ctr_clex_load( buffer );
	token = ctr_clex_tok();
	ctr_test(token == CTR_TOKEN_COLON);
	ctr_test(strlen(ctr_clex_tok_value())==0);
	ctr_test(strcmp(ctr_clex_tok_describe(token),CTR_DICT_PARAMETER_PREFIX)==0);
	free(buffer);
	buffer = calloc(40,1);
	sprintf( buffer, "%s", CTR_DICT_RETURN );
	ctr_clex_load( buffer );
	token = ctr_clex_tok();
	ctr_test(token == CTR_TOKEN_RET);
	ctr_test(strlen(ctr_clex_tok_value())==0);
	ctr_test(strcmp(ctr_clex_tok_describe(token),CTR_DICT_RETURN)==0);
	free(buffer);
	buffer = calloc(40,1);
	sprintf( buffer, "%s", CTR_DICT_ASSIGN );
	ctr_clex_load( buffer );
	token = ctr_clex_tok();
	ctr_test(token == CTR_TOKEN_ASSIGNMENT);
	ctr_test(strlen(ctr_clex_tok_value())==0);
	ctr_test(strcmp(ctr_clex_tok_describe(token),CTR_DICT_ASSIGN)==0);
	free(buffer);
	buffer = calloc(40,1);
	ctr_program_length = 0;
	ctr_clex_load( buffer );
	token = ctr_clex_tok();
	ctr_test(token == CTR_TOKEN_FIN);
	ctr_test(strlen(ctr_clex_tok_value())==0);
	ctr_test(strcmp(ctr_clex_tok_describe(token),"end of program")==0);
	free(buffer);
}

/**
 * Run Core tests.
 */
void ctr_coretest() {
	printf("Running Internal Tests\n");
	ctr_coretest_tokens();
	exit(0);
}

/**
 * Start internal unit tests.
 */
ctr_object* ctr_program_test( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_coretest();
	return CtrStdNil;
}