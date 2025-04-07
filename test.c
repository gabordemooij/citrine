#include <stdio.h>
#include <string.h>
#include "citrine.h"

#ifdef INCLUDETESTS
 
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
	sprintf( buffer, "%s", CTR_DICT_PAREN_CLOSE );
	ctr_clex_load( buffer );
	token = ctr_clex_tok();
	ctr_test(token == CTR_TOKEN_PARCLOSE);
	ctr_test(strlen(ctr_clex_tok_value())==0);
	ctr_test(strcmp(ctr_clex_tok_describe(token),CTR_DICT_PAREN_CLOSE)==0);
	sprintf( buffer, "%s", CTR_DICT_BLOCK_START );
	ctr_clex_load( buffer );
	token = ctr_clex_tok();
	ctr_test(token == CTR_TOKEN_BLOCKOPEN);
	ctr_test(strlen(ctr_clex_tok_value())==0);
	ctr_test(strcmp(ctr_clex_tok_describe(token),CTR_DICT_BLOCK_START)==0);
	sprintf( buffer, "%s", CTR_DICT_BLOCK_END );
	ctr_clex_load( buffer );
	token = ctr_clex_tok();
	ctr_test(token == CTR_TOKEN_BLOCKCLOSE);
	ctr_test(strlen(ctr_clex_tok_value())==0);
	ctr_test(strcmp(ctr_clex_tok_describe(token),CTR_DICT_BLOCK_END)==0);
	sprintf( buffer, "%s", CTR_DICT_END_OF_LINE );
	ctr_clex_load( buffer );
	token = ctr_clex_tok();
	ctr_test(token == CTR_TOKEN_DOT);
	ctr_test(strlen(ctr_clex_tok_value())==0);
	ctr_test(strcmp(ctr_clex_tok_describe(token),CTR_DICT_END_OF_LINE)==0);
	sprintf( buffer, "%s", CTR_DICT_MESSAGE_CHAIN );
	ctr_clex_load( buffer );
	token = ctr_clex_tok();
	ctr_test(token == CTR_TOKEN_CHAIN);
	ctr_test(strlen(ctr_clex_tok_value())==0);
	ctr_test(strcmp(ctr_clex_tok_describe(token),CTR_DICT_MESSAGE_CHAIN)==0);
	sprintf( buffer, "%s", CTR_DICT_PARAMETER_PREFIX );
	ctr_clex_load( buffer );
	token = ctr_clex_tok();
	ctr_test(token == CTR_TOKEN_COLON);
	ctr_test(strlen(ctr_clex_tok_value())==0);
	ctr_test(strcmp(ctr_clex_tok_describe(token),CTR_DICT_PARAMETER_PREFIX)==0);
	sprintf( buffer, "%s ", CTR_DICT_RETURN );
	ctr_clex_load( buffer );
	token = ctr_clex_tok();
	
	ctr_test(token == CTR_TOKEN_RET);
	ctr_test(strlen(ctr_clex_tok_value())==0);
	ctr_test(strcmp(ctr_clex_tok_describe(token),"↲")==0);
	sprintf( buffer, "%s", CTR_DICT_ASSIGN );
	ctr_clex_load( buffer );
	
	token = ctr_clex_tok();
	ctr_test(token == CTR_TOKEN_ASSIGNMENT);
	ctr_test(strlen(ctr_clex_tok_value())==0);
	ctr_test(strcmp(ctr_clex_tok_describe(token),"≔")==0);
	ctr_program_length = 0;
	ctr_clex_load( buffer );
	token = ctr_clex_tok();
	ctr_test(token == CTR_TOKEN_FIN);
	ctr_test(strlen(ctr_clex_tok_value())==0);
	ctr_test(strcmp(ctr_clex_tok_describe(token),"end of program")==0);
	free(buffer);
}

/**
 * Perform parser test
 */
void ctr_coretest_parser() {
	char* buffer;
	ctr_program_length = 40;
	buffer = calloc(40,1);
	sprintf(buffer, ">> x := 1.");
	ctr_clex_load(buffer);
	ctr_tnode* tree_node = ctr_cparse_expr(0);
	ctr_test(tree_node->type == CTR_AST_NODE_EXPRASSIGNMENT);
	ctr_test(tree_node->nodes->node->type == CTR_AST_NODE_REFERENCE);
	ctr_test(tree_node->nodes->node->vlen == 1);
	ctr_test(strcmp(tree_node->nodes->node->value,"x")==0);
	ctr_test(tree_node->nodes->next->node->type == CTR_AST_NODE_LTRNUM);
	free(buffer);
}

/**
 * Test memory functions.
 */
void ctr_coretest_memory() {
	char* chunk;
	size_t size;
	size_t expected_size;
	chunk = ctr_heap_allocate(10);
	size = (size_t) *((size_t*) ((char*)chunk - sizeof(size_t)));
	expected_size = (ctr_gc_mode & 8) ? 32 : (10 + sizeof(size_t));
	ctr_test(size == expected_size);
	chunk = ctr_heap_allocate(100);
	size = (size_t) *((size_t*) ((char*)chunk - sizeof(size_t)));
	expected_size = (ctr_gc_mode & 8) ? 128 : (100 + sizeof(size_t));
	ctr_test(size == expected_size);
	chunk = ctr_heap_allocate(32);
	size = (size_t) *((size_t*) ((char*)chunk - sizeof(size_t)));
	expected_size = (ctr_gc_mode & 8) ? 64 : (32 + sizeof(size_t));
	ctr_test(size == expected_size);
	chunk = ctr_heap_allocate((32 - sizeof(size_t)));
	size = (size_t) *((size_t*) ((char*)chunk - sizeof(size_t)));
	expected_size = (ctr_gc_mode & 8) ? 32 : 33;
	ctr_test(size == expected_size);
	/* heap allocator will not fail for size 0, will just return an empty block */
	/* btw, also not fail for -1 = just a huge block (unsigned!) */
	chunk = ctr_heap_allocate(0);
	size = (size_t) *((size_t*) ((char*)chunk - sizeof(size_t)));
	expected_size = (ctr_gc_mode & 8) ? 32 : sizeof(size_t);
	ctr_test(size == expected_size);
	/* heap free will not crash on NULL */
	ctr_heap_free(NULL);
}

/**
 * Run Core tests.
 */
void ctr_coretest() {
	printf("Running Internal Tests\n");
	ctr_coretest_tokens();
	ctr_coretest_parser();
	ctr_coretest_memory();
	exit(0);
}

#else

/**
 * No Core tests.
 */
void ctr_coretest() {
	printf("No Core Tests Included\n");
	exit(0);
}

#endif

/**
 * Start internal unit tests.
 */
ctr_object* ctr_program_test( ctr_object* myself, ctr_argument* argumentList ) {
	ctr_coretest();
	return CtrStdNil;
}