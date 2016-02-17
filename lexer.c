#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>

#include "citrine.h"


int ctr_clex_bflmt = 100;
ctr_size ctr_clex_tokvlen = 0; /* length of the string value of a token */
char* ctr_clex_buffer;
char* ctr_code;
char* ctr_eofcode;
char* ctr_clex_oldptr;
char* ctr_clex_olderptr;
int       ctr_clex_verbatim_mode = 0;              /* flag: indicates whether lexer operates in verbatim mode or not (1 = ON, 0 = OFF) */
uintptr_t ctr_clex_verbatim_mode_insert_quote = 0; /* pointer to 'overlay' the 'fake quote' for verbatim mode */

/**
 * CTRLexerLoad
 *
 * Loads program into memory.
 */
void ctr_clex_load(char* prg) {
	ctr_code = prg;
	ctr_clex_buffer = malloc(ctr_clex_bflmt);
	ctr_clex_buffer[0] = '\0';
	ctr_eofcode = (ctr_code + ctr_program_length - 1);
}

/**
 * CTRLexerTokenValue
 *
 * Returns the string of characters representing the value
 * of the currently selected token.
 */
char* ctr_clex_tok_value() {
	return ctr_clex_buffer;
}

/**
 * CTRLexerTokenValueLength
 *
 * Returns the length of the value of the currently selected token.
 */
long ctr_clex_tok_value_length() {
	return ctr_clex_tokvlen;
}

/**
 * CTRLexerPutBackToken
 *
 * Puts back a token and resets the pointer to the previous one.
 */
void ctr_clex_putback() {
	ctr_code = ctr_clex_oldptr;
	ctr_clex_oldptr = ctr_clex_olderptr;
}

/**
 * CTRLexerReadToken
 *
 * Reads the next token from the program buffer and selects this
 * token.
 */
int ctr_clex_tok() {
	char c;
	int i, comment_mode;
	ctr_clex_tokvlen = 0;
	ctr_clex_olderptr = ctr_clex_oldptr;
	ctr_clex_oldptr = ctr_code;
	i = 0;
	comment_mode = 0;

	/* if verbatim mode is on and we passed the '>' verbatim write message, insert a 'fake quote' (?>') */
	if (ctr_clex_verbatim_mode == 1 && ctr_clex_verbatim_mode_insert_quote == (uintptr_t) ctr_code) {
		return CTR_TOKEN_QUOTE;
	}

	c = *ctr_code;
	while(ctr_code != ctr_eofcode && (isspace(c) || c == '#' || comment_mode)) {
		if (c == '\n') comment_mode = 0;
		if (c == '#') comment_mode = 1;
		ctr_code ++;
		c = *ctr_code;
	}
	if (ctr_code == ctr_eofcode) return CTR_TOKEN_FIN;
	if (c == '(') { ctr_code++; return CTR_TOKEN_PAROPEN; }
	if (c == ')') { ctr_code++; return CTR_TOKEN_PARCLOSE; }
	if (c == '{') { ctr_code++; return CTR_TOKEN_BLOCKOPEN; }
	if (c == '}') { ctr_code++; return CTR_TOKEN_BLOCKCLOSE; }
	if (c == '.') { ctr_code++; return CTR_TOKEN_DOT; }
	if (c == ',') { ctr_code++; return CTR_TOKEN_CHAIN; }
	if (c == ':' && (ctr_code+1)<ctr_eofcode && (*(ctr_code+1)=='=')) {
		ctr_code += 2;
		return CTR_TOKEN_ASSIGNMENT; 
	}
	if (c == ':') { ctr_code++; return CTR_TOKEN_COLON; }
	if (c == '^') { ctr_code++; return CTR_TOKEN_RET; }
	if (c == '\'') { ctr_code++; return CTR_TOKEN_QUOTE; }
	if ((c == '-' && (ctr_code+1)<ctr_eofcode && isdigit(*(ctr_code+1))) || isdigit(c)) {
		ctr_clex_buffer[i] = c; ctr_clex_tokvlen++;
		i++;
		ctr_code++;
		c = *ctr_code;
		while((isdigit(c))) {
			ctr_clex_buffer[i] = c; ctr_clex_tokvlen++;
			i++;
			ctr_code++;
			c = *ctr_code;
		}
		if (c=='.' && (ctr_code+1 <= ctr_eofcode) && !isdigit(*(ctr_code+1))) {
			return CTR_TOKEN_NUMBER;
		}
		if (c=='.') {
			ctr_clex_buffer[i] = c; ctr_clex_tokvlen++;
			i++;
			ctr_code++;
			c = *ctr_code;
		}
		while((isdigit(c))) {
			ctr_clex_buffer[i] = c; ctr_clex_tokvlen++;
			i++;
			ctr_code++;
			c = *ctr_code;
		}
		return CTR_TOKEN_NUMBER;
	}
	if (strncmp(ctr_code, "True", 4)==0){
		if (CTR_IS_DELIM(*(ctr_code + 4))) { 
			ctr_code += 4;
			return CTR_TOKEN_BOOLEANYES;
		}
	}
	if (strncmp(ctr_code, "False", 5)==0){
		if (CTR_IS_DELIM(*(ctr_code + 5))) { 
			ctr_code += 5;
			return CTR_TOKEN_BOOLEANNO;
		}
	}
	if (strncmp(ctr_code, "Nil", 3)==0){
		if (CTR_IS_DELIM(*(ctr_code + 3))) {
			ctr_code += 3;
			return CTR_TOKEN_NIL;
		}
	}

	/* if we encounter a '?>' sequence, switch to verbatim mode in lexer */
	if (strncmp(ctr_code, "?>", 2)==0){
		ctr_clex_verbatim_mode = 1;
	}

	/* if lexer is in verbatim mode and we pass the '>' symbol insert a fake quote as next token */
	if (strncmp(ctr_code, ">", 1)==0 && ctr_clex_verbatim_mode == 1) {
		ctr_clex_verbatim_mode_insert_quote = (uintptr_t) (ctr_code+1); /* this way because multiple invocations should return same result */
	}

	/*
	 * these symbols are special because we often like to use
	 * them without spacing: 1+2 instead of 1 + 2.
	 */
	if (c=='+' || c=='-' || c=='/' || c=='*') {
		ctr_code++; ctr_clex_tokvlen = 1; ctr_clex_buffer[0] = c; return CTR_TOKEN_REF;
	}

	/*
	 * these are also special, they are easy notations for unicode symbols.
	 * we also return directly because we would like to use them without spaces as well: 1>=2...
	 */
	if ((ctr_code+1)<ctr_eofcode) {
		if (((char)*(ctr_code) == '>') && ((char)*(ctr_code+1)=='=')){
			ctr_code +=2; ctr_clex_tokvlen = 3; memcpy(ctr_clex_buffer, "≥", 3); return CTR_TOKEN_REF;
		}
		if (((char)*(ctr_code) == '<') && ((char)*(ctr_code+1)=='=')){
			ctr_code +=2; ctr_clex_tokvlen = 3; memcpy(ctr_clex_buffer, "≤", 3); return CTR_TOKEN_REF;
		}
		if (((char)*(ctr_code) == '!') && ((char)*(ctr_code+1)=='=')){
			ctr_code +=2; ctr_clex_tokvlen = 3; memcpy(ctr_clex_buffer, "≠", 3); return CTR_TOKEN_REF;
		}
		if (((char)*(ctr_code) == '<') && ((char)*(ctr_code+1)=='-')){
			ctr_code +=2; ctr_clex_tokvlen = 3; memcpy(ctr_clex_buffer, "←", 3); return CTR_TOKEN_REF;
		}
		/* be very nice, accidental == will be converted to = */
		if (((char)*(ctr_code) == '=') && ((char)*(ctr_code+1)=='=')){
			ctr_code +=2; ctr_clex_tokvlen = 1; memcpy(ctr_clex_buffer, "=", 1); return CTR_TOKEN_REF;
		}
	}

	/* later because we tolerate == as well. */
	if (c=='=' || c=='>' || c =='<') {
		ctr_code++; ctr_clex_tokvlen = 1; ctr_clex_buffer[0] = c; return CTR_TOKEN_REF;
	}

	if (c == '|' || c == '\\') { ctr_code++; return CTR_TOKEN_BLOCKPIPE; }

	while(!isspace(c) && CTR_IS_NO_TOK(c) && ctr_code!=ctr_eofcode
	&& c != '+' && c!='*' && c!='/' && c!='=' && c!='>' && c!='<' && c!='&' /* and c is not one of the special symbols */
	) {
		ctr_clex_buffer[i] = c; ctr_clex_tokvlen++;
		i++;
		if (i > ctr_clex_bflmt) {
			printf("[ERROR L001]: Token Buffer Exausted. Tokens may not exceed 100 bytes.");
			exit(1);
		}
		ctr_code++;
		c = *ctr_code;
	}

	return CTR_TOKEN_REF;
}


/**
 * CTRLexerStringReader
 *
 * Reads an entire string between a pair of quotes.
 */
char* ctr_clex_readstr() {
	char* strbuff;
	char c;
	long memblock = 40;
	int escape;
	char* beginbuff;
	long page = 100; /* 100 byte pages */
	ctr_clex_tokvlen=0;
	strbuff = (char*) malloc(memblock);
	c = *ctr_code;
	escape = 0;
	beginbuff = strbuff;
	while(
		(   /* reading string in non-verbatim mode, read until the first non-escaped quote */
			ctr_clex_verbatim_mode == 0 &&
			(
				c != '\'' ||
				escape == 1
			)
		)
		||
		(   /* reading string in verbatim mode, read until the '<?' sequence */
			ctr_clex_verbatim_mode == 1
			&&
			!(
				c == '<' &&
				((ctr_code+1) < ctr_eofcode) &&
				*(ctr_code+1) == '?'
			)
			&&
			(ctr_code < ctr_eofcode)
		)
	) {
		if (c == 'n' && escape == 1) {
			c = '\n';
		}
		if (c == '\\' && escape == 0 && ctr_clex_verbatim_mode == 0) {
			escape = 1;
			ctr_code++;
			c = *ctr_code;
			continue;
		}
		ctr_clex_tokvlen ++;
		if (ctr_clex_tokvlen > memblock) {
			memblock += page;
			beginbuff = (char*) ctr_realloc(beginbuff, memblock, (memblock-page), 0);
			if (beginbuff == NULL) {
				printf("Out of memory\n");
				exit(1);
			}
			/* reset pointer, memory location might have been changed */
			strbuff = beginbuff + (ctr_clex_tokvlen -1);
		}
		escape = 0;
		*(strbuff) = c;
		strbuff++;
		ctr_code++;
		c = *ctr_code;
	}
	if (ctr_clex_verbatim_mode) {
		if (ctr_code >= ctr_eofcode) { /* if we reached EOF in verbatim mode, append closing sequence '<?.' */
			strncpy(ctr_code,"<?.", 3);
			ctr_eofcode += 3;
		}
		ctr_code++; /* in verbatim mode, hop over the trailing ? as well */
	}
	ctr_clex_verbatim_mode = 0;               /* always turn verbatim mode off */
	ctr_clex_verbatim_mode_insert_quote = 0;  /* erase verbatim mode pointer overlay for fake quote */
	return beginbuff;
}
