
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>

#include "uthash.h"
#include "citrine.h"


typedef struct {
	int tid;
	char* value;
} tk;


int bflmt = 100;
long tokvlen = 0; //length of the string value of a token
char* buffer;
char* code;
char* codePoint;
char* eofcode;
char* oldptr;
char* olderptr;
int flag_operator = 0;

void clex_load(char* prg) {
	code = prg;
	buffer = malloc(bflmt);
	buffer[0] = '\0';
	eofcode = (code + strlen(code));
}

char* clex_tok_value() {
	return buffer;
}

long clex_tok_value_length() {
	return tokvlen;
}

void clex_putback() {
	code = oldptr;
	oldptr = olderptr;
}

int clex_tok() {
	tokvlen = 0;
	olderptr = oldptr;
	oldptr = code;
	char c;
	int i = 0;
	int comment_mode = 0;
	c = *code;
	while(code != eofcode && (isspace(c) || c == '#' || comment_mode)) {
		if (c == '\n') comment_mode = 0;
		if (c == '#') comment_mode = 1;
		code ++;
		c = *code;
	}
	if (code == eofcode) return FIN;
	if (c == '(') { code++; return PAROPEN; }
	if (c == ')') { code++; return PARCLOSE; }
	if (c == '{') { code++; return BLOCKOPEN; }
	if (c == '}') {  code++; return BLOCKCLOSE; }
	
	if (c == '.') { code++; return DOT; }
	if (c == ',') { code++; return CHAIN; }
	if (c == ':') { code++; return COLON; }
	if (c == '=' && (code+1)<eofcode && (*(code+1)!='=')) { 
		code++;
		return ASSIGNMENT; 
	}
	if (c == '^') { code++; return RET; }
	if (c == '\'') { code++; return QUOTE; }
	if ((c == '-' && (code+1)<eofcode && isdigit(*(code+1))) || isdigit(c)) {
		buffer[i] = c; tokvlen++;
		i++;
		code++;
		c = *code;
		while((isdigit(c))) {
			buffer[i] = c; tokvlen++;
			i++;
			code++;
			c = *code;
		}
		if (c=='.' && (code+1 <= eofcode) && !isdigit(*(code+1))) {
			return NUMBER;
		}
		if (c=='.') {
			buffer[i] = c; tokvlen++;
			i++;
			code++;
			c = *code;
		}
		while((isdigit(c))) {
			buffer[i] = c; tokvlen++;
			i++;
			code++;
			c = *code;
		}
		return NUMBER;
	}
	if (strncmp(code, "True", 4)==0){
		if (CTR_IS_DELIM(*(code + 4))) { 
			code += 4;
			return BOOLEANYES;
		}
	}
	if (strncmp(code, "False", 5)==0){
		if (CTR_IS_DELIM(*(code + 5))) { 
			code += 5;
			return BOOLEANNO;
		}
	}
	if (strncmp(code, "Nil", 3)==0){
		if (CTR_IS_DELIM(*(code + 3))) {
			code += 3;
			return NIL;
		}
	}
	if (strncmp(code, ">=", 2)==0){
		code += 2;
		tokvlen = 2; strncpy(buffer, ">=", 2);
		return REF;
	}
	if (strncmp(code, "<=", 2)==0){
		code += 2;
		tokvlen = 2; strncpy(buffer, "<=", 2);
		return REF;
	}
	
	if (strncmp(code, "==", 2)==0){
		code += 2;
		tokvlen = 2; strncpy(buffer, "==", 2);
		return REF;
	}
	
	if (strncmp(code, "!=", 2)==0){
		code += 2;
		tokvlen = 2; strncpy(buffer, "!=", 2);
		return REF;
	}
	
	if (strncmp(code, "||", 2)==0){
		code += 2;
		tokvlen = 2; strncpy(buffer, "||", 2);
		return REF;
	}
	
	if (c == '|' || c == '\\') { code++; return BLOCKPIPE; }
	
	if (strncmp(code, "&&", 2)==0){
		code += 2;
		tokvlen = 2; strncpy(buffer, "&&", 2);
		return REF;
	}

	if (strncmp(code, "*", 1)==0){
		code += 1;
		tokvlen = 1; strncpy(buffer, "*", 1);
		return REF;
	}

	if (strncmp(code, "/", 1)==0){
		code += 1;
		tokvlen = 1; strncpy(buffer, "/", 1);
		return REF;
	}

	if (strncmp(code, "+", 1)==0){
		code += 1;
		tokvlen = 1; strncpy(buffer, "+", 1);
		return REF;
	}

	if (strncmp(code, "-", 1)==0){
		code += 1;
		tokvlen = 1; strncpy(buffer, "-", 1);
		return REF;
	}

	while(!isspace(c) && CTR_IS_NO_TOK(c) && code!=eofcode) {
		buffer[i] = c; tokvlen++;
		i++;
		if (i > bflmt) {
			printf("[ERROR L001]: Token Buffer Exausted. Tokens may not exceed 100 bytes.");
			exit(1);
		}
		code++;
		c = *code;
	}

	return REF;
}


char* clex_readstr() {
	clex_len=0;
	char* strbuff = malloc(100);
	char c = *code;
	int escape = 0;
	char* beginbuff = strbuff;
	while((c != '\'' || escape == 1)) {
		if (c == '\\' && escape == 0) {
			escape = 1;
			code++;
			c = *code;
			continue;
		}
		clex_len ++;
		escape = 0;
		*strbuff = c;
		strbuff++;
		code++;
		c = *code;
	}
	*strbuff = '\0';
	return beginbuff;
}
