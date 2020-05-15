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
char* ctr_code_start;
char* ctr_code_eoi;

int ctr_clex_line_number;
char* ctr_eofcode;

char* ctr_clex_oldptr;
char* ctr_clex_olderptr;
int ctr_clex_old_line_number = 0;
int ctr_clex_ignore_modes = 0;

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
char* ctr_clex_desc_tok_ret_unicode = "↲";
char* ctr_clex_desc_tok_fin = "end of program";
char* ctr_clex_desc_tok_unknown = "(unknown token)";

char* ctr_clex_keyword_me_icon;
char* ctr_clex_keyword_my_icon;
char* ctr_clex_keyword_var_icon;
ctr_size ctr_clex_keyword_my_icon_len;
ctr_size ctr_clex_keyword_var_icon_len;

ctr_size ctr_clex_keyword_qo_len;
ctr_size ctr_clex_keyword_qc_len;

int ctr_clex_true_len = 0;
int ctr_clex_false_len = 0;
int ctr_clex_nil_len = 0;

char* ivarname;
int ivarlen;

/**
 * Lexer - is Symbol Delimiter ?
 * Determines whether the specified symbol is a delimiter.
 * Returns 1 if the symbol is a delimiter and 0 otherwise.
 *
 * @param char* code code to be inspected
 *
 * @return uint8_t
 */
uint8_t ctr_clex_is_delimiter( char* code ) {
	if (strncmp(code, CTR_DICT_END_OF_LINE, ctr_clex_keyword_eol_len) == 0) {
		return 1;
	}
	char symbol = *(code);
	return (
	   symbol == '('
	|| symbol == ')'
	|| symbol == ','
	|| symbol == ':'
	|| symbol == ' ' );
}

/**
 * CTRLexerEmitError
 *
 * Displays an error message for the lexer.
 */
void ctr_clex_emit_error( char* message ) {
	printf(CTR_ERR_LEX, message, ctr_clex_line_number );
	exit(1);
}

/**
 * CTRLexerLoad
 *
 * Loads program into memory.
 */
void ctr_clex_load(char* prg) {
	
	ctr_clex_true_len = strlen(CTR_DICT_TRUE);
	ctr_clex_false_len = strlen(CTR_DICT_FALSE);
	ctr_clex_nil_len = strlen(CTR_DICT_NIL);

	ctr_code = prg;
	ctr_code_start = prg;
	ctr_clex_buffer = ctr_heap_allocate_tracked(ctr_clex_bflmt);
	ctr_clex_buffer[0] = '\0';
	ctr_eofcode = (ctr_code + ctr_program_length);
	ctr_clex_line_number = 0;
	/* skip the first line if it starts with a #. */
	if (*ctr_code_start=='#') {
		while(ctr_code<ctr_eofcode && *ctr_code!='\n') {
			ctr_code++;
		}
	}
	ctr_code_start = ctr_code;
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
			description = ctr_clex_desc_tok_ret_unicode;
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
	int i;
	char eol;
	char qo;
	ctr_clex_tokvlen = 0;
	ctr_clex_olderptr = ctr_clex_oldptr;
	ctr_clex_oldptr = ctr_code;
	ctr_clex_old_line_number = ctr_clex_line_number;
	i = 0;
	c = *ctr_code;
	while(ctr_code != ctr_eofcode && (isspace(c))) {
		if (c == '\n') ctr_clex_line_number++;
		ctr_code ++;
		c = *ctr_code;
	}
	if (ctr_code == ctr_eofcode) return CTR_TOKEN_FIN;
	if (c == '(') { ctr_code++; return CTR_TOKEN_PAROPEN; }
	if (c == ')') { ctr_code++; return CTR_TOKEN_PARCLOSE; }
	if (c == '{') { ctr_code++; return CTR_TOKEN_BLOCKOPEN; }
	if (c == '}') { ctr_code++; return CTR_TOKEN_BLOCKCLOSE; }
	if (strncmp(ctr_code, CTR_DICT_END_OF_LINE, ctr_clex_keyword_eol_len)==0) {
		ctr_code+=ctr_clex_keyword_eol_len;
		return CTR_TOKEN_DOT;
	}
	if (c == ',') { ctr_code++; return CTR_TOKEN_CHAIN; }
	if (c == ':' && (ctr_code+1)<ctr_eofcode && (*(ctr_code+1)=='=')) {
		ctr_code += 2;
		return CTR_TOKEN_ASSIGNMENT; 
	}
	if (c == ':') { ctr_code++; return CTR_TOKEN_COLON; }
	if ( ( ctr_code + 2) < ctr_eofcode
		&&   (uint8_t)            c == 0xE2
		&& ( (uint8_t) *(ctr_code+1)==0x86)
		&& ( (uint8_t) *(ctr_code+2)==0xB2)  ) {
		ctr_code += 3;
		return CTR_TOKEN_RET;
	}

	if (strncmp(ctr_code, CTR_DICT_QUOT_OPEN, ctr_clex_keyword_qo_len)==0) {
		ctr_code+=ctr_clex_keyword_qo_len;
		return CTR_TOKEN_QUOTE;
	}
	
	eol = ( strncmp(ctr_code,CTR_DICT_END_OF_LINE,ctr_clex_keyword_eol_len)==0 );
	
	if ((c == '-' && (ctr_code+1)<ctr_eofcode && isdigit(*(ctr_code+1))) || isdigit(c)) {
		ctr_clex_buffer[i] = c; ctr_clex_tokvlen++;
		i++;
		ctr_code++;
		c = *ctr_code;
		while(
			(isdigit(c)) 
			|| 
			(
				((ctr_code+ctr_clex_keyword_num_sep_tho_len+1) <= ctr_eofcode) &&
				(strncmp(ctr_code,CTR_DICT_NUM_THO_SEP,ctr_clex_keyword_num_sep_tho_len)==0) &&
				(isdigit(*(ctr_code+ctr_clex_keyword_num_sep_tho_len)))
			)
		) {
			//Parse thousands separator (filter)
			if (!(strncmp(ctr_code,CTR_DICT_NUM_THO_SEP,ctr_clex_keyword_num_sep_tho_len)==0)) {
				ctr_clex_buffer[i] = c; ctr_clex_tokvlen++;
				i++;
			}
			ctr_code++;
			c = *ctr_code;
		}
		eol = ( strncmp(ctr_code,CTR_DICT_END_OF_LINE,ctr_clex_keyword_eol_len)==0 );
		if (eol && (ctr_code+ctr_clex_keyword_eol_len <= ctr_eofcode) && !isdigit(*(ctr_code+ctr_clex_keyword_eol_len))) {
			return CTR_TOKEN_NUMBER;
		}
		
		//Parse decimal separator (turn into international symbol .)
		if (
		((ctr_code+ctr_clex_keyword_num_sep_dec_len+1) <= ctr_eofcode) &&
		isdigit(*(ctr_code+ctr_clex_keyword_num_sep_dec_len)) &&
		strncmp(ctr_code,CTR_DICT_NUM_DEC_SEP,ctr_clex_keyword_num_sep_dec_len)==0) {
			ctr_clex_buffer[i] = '.'; ctr_clex_tokvlen++;
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
	if (strncmp(ctr_code, CTR_DICT_TRUE, ctr_clex_true_len)==0){
		if ( ctr_clex_is_delimiter( ( ctr_code + ctr_clex_true_len ) ) ) {
			ctr_code += ctr_clex_true_len;
			memcpy(ctr_clex_buffer, CTR_DICT_TRUE, ctr_clex_true_len);
			ctr_clex_tokvlen = ctr_clex_true_len;
			return CTR_TOKEN_BOOLEANYES;
		}
	}
	if (strncmp(ctr_code, CTR_DICT_FALSE, ctr_clex_false_len)==0){
		if ( ctr_clex_is_delimiter( ( ctr_code + ctr_clex_false_len ) ) ) {
			ctr_code += ctr_clex_false_len;
			memcpy(ctr_clex_buffer, CTR_DICT_FALSE, ctr_clex_false_len);
			ctr_clex_tokvlen = ctr_clex_false_len;
			return CTR_TOKEN_BOOLEANNO;
		}
	}
	if (strncmp(ctr_code, CTR_DICT_NIL, ctr_clex_nil_len)==0){
		if ( ctr_clex_is_delimiter( ( ctr_code + ctr_clex_nil_len ) ) ) {
			ctr_code += ctr_clex_nil_len;
			memcpy(ctr_clex_buffer, CTR_DICT_NIL, ctr_clex_nil_len);
			ctr_clex_tokvlen = ctr_clex_nil_len;
			return CTR_TOKEN_NIL;
		}
	}

	qo  = ( strncmp(ctr_code, CTR_DICT_QUOT_OPEN, ctr_clex_keyword_qo_len)==0 );
	while(
	!isspace(c) && (
		c != '(' &&
		c != ')' &&
		c != '{' &&
		c != '}' &&
		!eol &&
		!qo  &&
		c !=','  &&
		( !(
		( ctr_code + 2) < ctr_eofcode
			&&   (uint8_t)            c == 226
			&& ( (uint8_t) *(ctr_code+1)== 134)
			&& ( (uint8_t) *(ctr_code+2)== 145) ) ) &&
		c != ':'
	)
	&& ctr_code!=ctr_eofcode
	) {
		ctr_clex_buffer[i] = c; ctr_clex_tokvlen++;
		i++;
		if (i > ctr_clex_bflmt) {
			ctr_clex_emit_error( CTR_ERR_TOKBUFF );
		}
		ctr_code++;
		c = *ctr_code;
		eol = ( strncmp(ctr_code,CTR_DICT_END_OF_LINE,ctr_clex_keyword_eol_len)==0 );
		qo  = ( strncmp(ctr_code, CTR_DICT_QUOT_OPEN, ctr_clex_keyword_qo_len)==0 );
	}
	return CTR_TOKEN_REF;
}

char* ctr_clex_code_pointer() {
	return ctr_code;
}

/**
 * CTRLexerStringReader
 *
 * Reads an entire string between a pair of quotes.
 */
char* ctr_clex_readstr() {
	ctr_size nesting = 0;
	char* strbuff;
	char c;
	int escape;
	char* beginbuff;
	ctr_size page = 100; /* 100 byte pages */
	ctr_size memblock = 100;
	ctr_clex_tokvlen=0;
	strbuff = (char*) ctr_heap_allocate(memblock);
	beginbuff = strbuff;
	escape = 0;
	/* ignore_modes is used for translator, then you dont want to interpret escape sequence but copy -verbatim- */
	while(
		(ctr_code<ctr_eofcode-ctr_clex_keyword_qc_len) &&
		(strncmp(ctr_code, CTR_DICT_QUOT_CLOSE, ctr_clex_keyword_qc_len)!=0 || nesting>0 || escape)
	) {
		c = *ctr_code;
		if ( c == '\n' ) ctr_clex_line_number ++;
		if ( c == '\\' && !escape) {
			escape = 1;
			if (ctr_clex_ignore_modes) {
				*(strbuff) = '\\';
				ctr_clex_tokvlen += 1;
				strbuff++;
			}
			ctr_code ++;
			continue;
		}
		if ( escape == 1 ) {
			switch(c) {
				case 'n':
					*(strbuff) = '\n';
					ctr_clex_tokvlen += 1;
					strbuff++;
					ctr_code++;
					break;
				case 'r':
					*(strbuff) = '\r';
					ctr_clex_tokvlen += 1;
					strbuff++;
					ctr_code++;
					break;
				case 't':
					*(strbuff) = '\t';
					ctr_clex_tokvlen += 1;
					strbuff++;
					ctr_code++;
					break;
				case 'v':
					*(strbuff) = '\v';
					ctr_clex_tokvlen += 1;
					strbuff++;
					ctr_code++;
					break;
				case 'b':
					*(strbuff) = '\b';
					ctr_clex_tokvlen += 1;
					strbuff++;
					ctr_code++;
					break;
				case 'a':
					*(strbuff) = '\a';
					ctr_clex_tokvlen += 1;
					strbuff++;
					ctr_code++;
					break;
				case 'f':
					*(strbuff) = '\f';
					ctr_clex_tokvlen += 1;
					strbuff++;
					ctr_code++;
					break;
				default:
					*(strbuff) = *(ctr_code);
					strbuff++;
					ctr_code++;
					ctr_clex_tokvlen++;
					break;
			}
		}
		else if (!ctr_clex_ignore_modes && !escape && strncmp(ctr_code, "↵", 3)==0) {
			ctr_code += 3;
			ctr_clex_tokvlen += 1;
			*(strbuff) = '\n';
			strbuff++;
		}
		else if (!ctr_clex_ignore_modes && !escape && strncmp(ctr_code, "⇿", 3)==0) {
			ctr_code += 3;
			ctr_clex_tokvlen += 1;
			*(strbuff) = '\t';
			strbuff++;
		}
		else if (strncmp(ctr_code, CTR_DICT_QUOT_CLOSE, ctr_clex_keyword_qc_len)==0) {
			if (!escape && !ctr_clex_ignore_modes) nesting--;
			strncpy(strbuff, CTR_DICT_QUOT_CLOSE, ctr_clex_keyword_qc_len);
			ctr_code += ctr_clex_keyword_qc_len;
			ctr_clex_tokvlen += ctr_clex_keyword_qc_len;
			strbuff += ctr_clex_keyword_qc_len;
		}
		else if (strncmp(ctr_code, CTR_DICT_QUOT_OPEN, ctr_clex_keyword_qo_len)==0) {
			if (!escape && !ctr_clex_ignore_modes) nesting++;
			strncpy(strbuff, CTR_DICT_QUOT_OPEN, ctr_clex_keyword_qo_len);
			ctr_code += ctr_clex_keyword_qo_len;
			ctr_clex_tokvlen += ctr_clex_keyword_qo_len;
			strbuff += ctr_clex_keyword_qo_len;
		} else {
			*(strbuff) = *(ctr_code);
			strbuff++;
			ctr_code++;
			ctr_clex_tokvlen++;
		}
		
		if ((ctr_clex_tokvlen + 10) >= memblock) {
			memblock += page;
			beginbuff = (char*) ctr_heap_reallocate( beginbuff, memblock );
			if (beginbuff == NULL) {
				ctr_clex_emit_error( CTR_ERR_OOM );
			}
			/* reset pointer, memory location might have been changed */
			strbuff = beginbuff + ctr_clex_tokvlen;
		}
		escape = 0;
	}
	/* absorb trailing quote, unless eof encountered - then string ends at eof */
	if (ctr_code<ctr_eofcode-ctr_clex_keyword_qc_len) {
		ctr_code += ctr_clex_keyword_qc_len;
	}
	return beginbuff;
}

void ctr_clex_set_ignore_modes( int ignore ) {
	ctr_clex_ignore_modes = ignore;
}

void ctr_clex_move_code_pointer(int movement) {
	ctr_code += movement;
}

int ctr_clex_forward_scan(char* e, ctr_size* newCodePointer) {
	char* bytes;
	char* eol = NULL;
	if (strncmp(CTR_DICT_END_OF_LINE,".",1)==0) {
		bytes = ":.,)";
	} else {
		bytes = ":,)";
		eol = CTR_DICT_END_OF_LINE;
	}
	ctr_size i = *(newCodePointer);
	int len = strlen(bytes);
	int nesting = 0;
	int blocks = 0;
	int quote = 0;
	int q;
	int escape = 0;
	int found = 0;
	while( (e+i) < ctr_eofcode ) {
		if (escape) escape = 0;
		if (!quote && *(e+i) == '(') nesting++;
		else if (!quote && nesting && *(e+i) == ')') nesting--;
		else if (!quote && *(e+i) == '{') blocks++;
		else if (!quote && blocks && *(e+i) == '}') blocks--;
		else if (!escape && !quote && strncmp((e+i),CTR_DICT_QUOT_OPEN, ctr_clex_keyword_qo_len) == 0) quote++;
		else if (!escape && quote && strncmp((e+i),CTR_DICT_QUOT_CLOSE, ctr_clex_keyword_qc_len) == 0) quote--;
		else if (quote && *(e+i) == '\\' && !escape) { escape = 1;  }
		else if (!nesting && !quote && !blocks) {
			for (q=0; q<len; q++) {
				if (*(e+i)==bytes[q]) {
					if (bytes[q]=='.' && isdigit(*(e+i-1)) && (e+i+1)<ctr_eofcode && isdigit(*(e+i+1))) continue;
					*(newCodePointer) = i;
					found = 1;
					break;
				}
				if (eol!=NULL && strncmp((e+i),eol,ctr_clex_keyword_eol_len)==0) {
					*(newCodePointer) = i;
					found = 1;
					break;
				}
			}
			if (found) break;
		}
		i++;
	}
	return found;
}

int ctr_clex_backward_scan( char* codePointer, char* bytes, ctr_size* offset, ctr_size limit ) {
	ctr_size q = *(offset);
	for(q=0; q<limit; q++) {
		if ((codePointer-q)<ctr_code_start) return 0;
		char backScanChar = *(codePointer-q);
		if (
			backScanChar == '\n'||
			backScanChar == '\t'||
			backScanChar == ' ' ||
			backScanChar == ')' ||
			backScanChar == '}'
		) {
			*(offset) = q;
			return 1;
		}
	}
	return 0;
}

