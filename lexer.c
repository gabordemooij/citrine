#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>

#include "citrine.h"


int ctr_clex_bflmt = 255;
ctr_size ctr_clex_tokvlen = 0; /* length of the string value of a token */
char* ctr_clex_buffer;
char* ctr_code;
char* ctr_code_eoi;
char* ctr_eofcode;
char* ctr_clex_oldptr;
char* ctr_clex_olderptr;
int       ctr_clex_verbatim_mode = 0;              /* flag: indicates whether lexer operates in verbatim mode or not (1 = ON, 0 = OFF) */
uintptr_t ctr_clex_verbatim_mode_insert_quote = 0; /* pointer to 'overlay' the 'fake quote' for verbatim mode */
int ctr_clex_old_line_number = 0;

char* ctr_clex_desc_tok_ref = "reference";
char* ctr_clex_desc_tok_quote = "'";
char* ctr_clex_desc_tok_number = "number";
char* ctr_clex_desc_tok_paropen = "(";
char* ctr_clex_desc_tok_parclose = ")";
char* ctr_clex_desc_tok_blockopen = "{";
char* ctr_clex_desc_tok_blockclose = "}";
char* ctr_clex_desc_tok_colon = ":";
char* ctr_clex_desc_tok_dot = ".";
char* ctr_clex_desc_tok_chain = ",";
char* ctr_clex_desc_tok_booleanyes = "True";
char* ctr_clex_desc_tok_booleanno = "False";
char* ctr_clex_desc_tok_nil = "Nil";
char* ctr_clex_desc_tok_assignment = ":=";
char* ctr_clex_desc_tok_ret = "^";
char* ctr_clex_desc_tok_ret_unicode = "↑";
char* ctr_clex_desc_tok_fin = "end of program";
char* ctr_clex_desc_tok_unknown = "(unknown token)";

int ctr_string_interpolation = 0;
char* ivarname;
int ivarlen;

/**
 * Lexer - is Symbol Delimiter ?
 * Determines whether the specified symbol is a delimiter.
 * Returns 1 if the symbol is a delimiter and 0 otherwise.
 *
 * @param char symbol symbol to be inspected
 *
 * @return uint8_t
 */
uint8_t ctr_clex_is_delimiter( char symbol ) {

	return (
	   symbol == '('
	|| symbol == ')'
	|| symbol == ','
	|| symbol == '.'
	|| symbol == ':'
	|| symbol == ' ' );
}

/**
 * CTRLexerEmitError
 *
 * Displays an error message for the lexer.
 */
void ctr_clex_emit_error( char* message ) {
	printf( "%s on line: %d. \n", message, ctr_clex_line_number );
	exit(1);
}

/**
 * CTRLexerLoad
 *
 * Loads program into memory.
 */
void ctr_clex_load(char* prg) {
	ctr_code = prg;
	ctr_clex_buffer = ctr_heap_allocate_tracked(ctr_clex_bflmt);
	ctr_clex_buffer[0] = '\0';
	ctr_eofcode = (ctr_code + ctr_program_length);
	ctr_clex_line_number = 0;
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


char* ctr_clex_tok_describe(int token)
{
	char* description;
	switch(token) {
		case CTR_TOKEN_RET:
			description = ctr_clex_desc_tok_ret;
			break;
		case CTR_TOKEN_ASSIGNMENT:
			description = ctr_clex_desc_tok_assignment;
			break;
		case CTR_TOKEN_BLOCKCLOSE:
			description = ctr_clex_desc_tok_blockclose;
			break;
		case CTR_TOKEN_BLOCKOPEN:
			description = ctr_clex_desc_tok_blockopen;
			break;
		case CTR_TOKEN_BOOLEANNO:
			description = ctr_clex_desc_tok_booleanno;
			break;
		case CTR_TOKEN_BOOLEANYES:
			description = ctr_clex_desc_tok_booleanyes;
			break;
		case CTR_TOKEN_CHAIN:
			description = ctr_clex_desc_tok_chain;
			break;
		case CTR_TOKEN_COLON:
			description = ctr_clex_desc_tok_colon;
			break;
		case CTR_TOKEN_DOT:
			description = ctr_clex_desc_tok_dot;
			break;
		case CTR_TOKEN_FIN:
			description = ctr_clex_desc_tok_fin;
			break;
		case CTR_TOKEN_NIL:
			description = ctr_clex_desc_tok_nil;
			break;
		case CTR_TOKEN_NUMBER:
			description = ctr_clex_desc_tok_number;
			break;
		case CTR_TOKEN_PARCLOSE:
			description = ctr_clex_desc_tok_parclose;
			break;
		case CTR_TOKEN_PAROPEN:
			description = ctr_clex_desc_tok_paropen;
			break;
		case CTR_TOKEN_QUOTE:
			description = ctr_clex_desc_tok_quote;
			break;
		case CTR_TOKEN_REF:
			description = ctr_clex_desc_tok_ref;
			break;
		default:
			description = ctr_clex_desc_tok_unknown;
	}
	return description;
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
	if ( ctr_string_interpolation > 0 ) {
		ctr_string_interpolation--;
		return;
	}
	ctr_code = ctr_clex_oldptr;
	ctr_clex_oldptr = ctr_clex_olderptr;
	ctr_clex_line_number = ctr_clex_old_line_number;
}

/**
 * CTRLexerReadToken
 *
 * Reads the next token from the program buffer and selects this
 * token.
 */
int ctr_clex_tok() {
	char c;
	int i, comment_mode, presetToken;
	ctr_clex_tokvlen = 0;
	ctr_clex_olderptr = ctr_clex_oldptr;
	ctr_clex_oldptr = ctr_code;
	ctr_clex_old_line_number = ctr_clex_line_number;
	i = 0;
	comment_mode = 0;

	/* a little state machine to handle string interpolation, */
	/* i.e. transforms ' $$x ' into: '' + x + ''. */
	switch( ctr_string_interpolation ) {
		case 1:
			presetToken = CTR_TOKEN_QUOTE;
			break;
		case 2:
		case 4:
			memcpy( ctr_clex_buffer, "+", 1 );
			ctr_clex_tokvlen = 1;
			presetToken = CTR_TOKEN_REF;
			break;
		case 3:
			memcpy( ctr_clex_buffer, ivarname, ivarlen );
			ctr_clex_tokvlen = ivarlen;
			presetToken = CTR_TOKEN_REF;
			break;
		case 5:
			ctr_code = ctr_code_eoi;
			presetToken = CTR_TOKEN_QUOTE;
			break;
	}
	/* return the preset token, and transition to next state */
	if ( ctr_string_interpolation ) {
		ctr_string_interpolation++;
		return presetToken;
	}

	/* if verbatim mode is on and we passed the '>' verbatim write message, insert a 'fake quote' (?>') */
	if (ctr_clex_verbatim_mode == 1 && ctr_clex_verbatim_mode_insert_quote == (uintptr_t) ctr_code) {
		return CTR_TOKEN_QUOTE;
	}

	c = *ctr_code;
	while(ctr_code != ctr_eofcode && (isspace(c) || c == '#' || comment_mode)) {
		if (c == '\n') {
			comment_mode = 0;
			ctr_clex_line_number++;
		}
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
	//↑
	if ( ( ctr_code + 2) < ctr_eofcode
		&&   (uint8_t)            c == 226
		&& ( (uint8_t) *(ctr_code+1)==134)
		&& ( (uint8_t) *(ctr_code+2)==145)  ) {
		ctr_code += 3;
		return CTR_TOKEN_RET;
	}
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
		if ( ctr_clex_is_delimiter( *( ctr_code + 4 ) ) ) {
			ctr_code += 4;
			return CTR_TOKEN_BOOLEANYES;
		}
	}
	if (strncmp(ctr_code, "False", 5)==0){
		if ( ctr_clex_is_delimiter( *( ctr_code + 5 ) ) ) {
			ctr_code += 5;
			return CTR_TOKEN_BOOLEANNO;
		}
	}
	if (strncmp(ctr_code, "Nil", 3)==0){
		if ( ctr_clex_is_delimiter( *( ctr_code + 3 ) ) ) {
			ctr_code += 3;
			return CTR_TOKEN_NIL;
		}
	}

	/* if we encounter a '?>' sequence, switch to verbatim mode in lexer */
	if (strncmp(ctr_code, "?>", 2)==0){
		ctr_clex_verbatim_mode = 1;
		ctr_code ++;
		memcpy(ctr_clex_buffer, "?", 1);
		ctr_clex_tokvlen = 1;
		return CTR_TOKEN_REF;
	}

	/* if lexer is in verbatim mode and we pass the '>' symbol insert a fake quote as next token */
	if (strncmp(ctr_code, ">", 1)==0 && ctr_clex_verbatim_mode == 1) {
		ctr_clex_verbatim_mode_insert_quote = (uintptr_t) (ctr_code+1); /* this way because multiple invocations should return same result */
		ctr_code ++;
		memcpy(ctr_clex_buffer, ">", 1);
		ctr_clex_tokvlen = 1;
		return CTR_TOKEN_REF;
	}

	while(
	!isspace(c) && (
		c != '#' &&
		c != '(' &&
		c != ')' &&
		c != '{' &&
		c != '}' &&
		c !='.'  &&
		c !=','  &&
		c !='^'  &&
		( !(
		( ctr_code + 2) < ctr_eofcode
			&&   (uint8_t)            c == 226
			&& ( (uint8_t) *(ctr_code+1)== 134)
			&& ( (uint8_t) *(ctr_code+2)== 145) ) ) &&
		c != ':' &&
		c != '\''
	) && ctr_code!=ctr_eofcode
	) {
		ctr_clex_buffer[i] = c; ctr_clex_tokvlen++;
		i++;
		if (i > ctr_clex_bflmt) {
			ctr_clex_emit_error( "Token Buffer Exausted. Tokens may not exceed 255 bytes" );
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
	long memblock = 1;
	int escape;
	char* beginbuff;
	long page = 100; /* 100 byte pages */
	size_t tracking_id;

	if ( ctr_string_interpolation == 6 ) {
		ctr_heap_free( ivarname );
		ctr_string_interpolation = 0;
	}

	ctr_clex_tokvlen=0;
	strbuff = (char*) ctr_heap_allocate_tracked(memblock);
	tracking_id = ctr_heap_get_latest_tracking_id();
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

		/* enter interpolation mode ( $x} ) */
		if (
			!ctr_clex_verbatim_mode &&
			!escape &&
			c == '$' &&
			((ctr_code+1) < ctr_eofcode) &&
			*(ctr_code+1) == '$'
		) {
			int q = 2;
			while( ( ctr_code + q ) < ctr_eofcode && !isspace(*(ctr_code + q)) && *(ctr_code + q) != '$' && *(ctr_code + q) != '\''  && q < 255 ) q++;
			if (isspace(*(ctr_code + q)) || *(ctr_code + q) == '$' || *(ctr_code + q) == '\'') {
				ivarname = ctr_heap_allocate( q );
				ivarlen  = q - 2;
				memcpy( ivarname, ctr_code + 2, q - 2 );
				ctr_string_interpolation = 1;
				ctr_code_eoi = ctr_code + q + 0; /* '$','$' and the name  ( name + 3 ) */
				break;
			}
		}

		if ( c == '\n' ) ctr_clex_line_number ++;

		if ( escape == 1 ) {
			switch(c) {
				case 'n':
					c = '\n';
					break;
				case 'r':
					c = '\r';
					break;
				case 't':
					c = '\t';
					break;
				case 'v':
					c = '\v';
					break;
				case 'b':
					c = '\b';
					break;
				case 'a':
					c = '\a';
					break;
				case 'f':
					c = '\f';
					break;
			}
		}

		if (c == '\\' && escape == 0 && ctr_clex_verbatim_mode == 0) {
			escape = 1;
			ctr_code++;
			c = *ctr_code;
			continue;
		}
		ctr_clex_tokvlen ++;
		if (ctr_clex_tokvlen >= memblock) {
			memblock += page;
			beginbuff = (char*) ctr_heap_reallocate_tracked( tracking_id, memblock );
			if (beginbuff == NULL) {
				ctr_clex_emit_error( "Out of memory" );
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
