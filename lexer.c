#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>

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
	eofcode = (code + ctr_program_length - 1);
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
	if (code == eofcode) return CTR_TOKEN_FIN;
	if (c == '(') { code++; return CTR_TOKEN_PAROPEN; }
	if (c == ')') { code++; return CTR_TOKEN_PARCLOSE; }
	if (c == '{') { code++; return CTR_TOKEN_BLOCKOPEN; }
	if (c == '}') {  code++; return CTR_TOKEN_BLOCKCLOSE; }
	
	if (c == '.') { code++; return CTR_TOKEN_DOT; }
	if (c == ',') { code++; return CTR_TOKEN_CHAIN; }
	if (c == ':' && (code+1)<eofcode && (*(code+1)=='=')) {
		code += 2;
		return CTR_TOKEN_ASSIGNMENT; 
	}
	if (c == ':') { code++; return CTR_TOKEN_COLON; }
	
	if (c == '^') { code++; return CTR_TOKEN_RET; }
	if (c == '\'') { code++; return CTR_TOKEN_QUOTE; }
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
			return CTR_TOKEN_NUMBER;
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
		return CTR_TOKEN_NUMBER;
	}
	if (strncmp(code, "True", 4)==0){
		if (CTR_IS_DELIM(*(code + 4))) { 
			code += 4;
			return CTR_TOKEN_BOOLEANYES;
		}
	}
	if (strncmp(code, "False", 5)==0){
		if (CTR_IS_DELIM(*(code + 5))) { 
			code += 5;
			return CTR_TOKEN_BOOLEANNO;
		}
	}
	if (strncmp(code, "Nil", 3)==0){
		if (CTR_IS_DELIM(*(code + 3))) {
			code += 3;
			return CTR_TOKEN_NIL;
		}
	}
	
	//these symbols are special because we often like to use
	//them without spacing: 1+2 instead of 1 + 2.
	if (c=='+' || c=='-' || c=='/' || c=='*') {
		code++; tokvlen = 1; buffer[i] = c; return CTR_TOKEN_REF;
	}
	//these are also special, they are easy notations for unicode symbols.
	//we also return directly because we would like to use them without spaces as well: 1>=2...
	if ((code+1)<eofcode) {
		if (((char)*(code) == '>') && ((char)*(code+1)=='=')){
			code +=2; tokvlen = 3; memcpy(buffer, "≥", 3); return CTR_TOKEN_REF;
		}
		if (((char)*(code) == '<') && ((char)*(code+1)=='=')){
			code +=2; tokvlen = 3; memcpy(buffer, "≤", 3); return CTR_TOKEN_REF;
		}
		if (((char)*(code) == '!') && ((char)*(code+1)=='=')){
			code +=2; tokvlen = 3; memcpy(buffer, "≠", 3); return CTR_TOKEN_REF;
		}
		if (((char)*(code) == '|') && ((char)*(code+1)=='|')){
			code +=2; tokvlen = 3; memcpy(buffer, "∨", 3); return CTR_TOKEN_REF;
		}
		if (((char)*(code) == '&') && ((char)*(code+1)=='&')){
			code +=2; tokvlen = 3; memcpy(buffer, "∧", 3); return CTR_TOKEN_REF;
		}
		if (((char)*(code) == '<') && ((char)*(code+1)=='-')){
			code +=2; tokvlen = 3; memcpy(buffer, "←", 3); return CTR_TOKEN_REF;
		}
		//be very nice, accidental == will be converted to =
		if (((char)*(code) == '=') && ((char)*(code+1)=='=')){
			code +=2; tokvlen = 1; memcpy(buffer, "=", 1); return CTR_TOKEN_REF;
		}
	}
	//later because we tolerate == as well.
	if (c=='=' || c=='>' || c =='<') {
		code++; tokvlen = 1; buffer[i] = c; return CTR_TOKEN_REF;
	}
	
	
	
	
	if (c == '|' || c == '\\') { code++; return CTR_TOKEN_BLOCKPIPE; }
	
	while(!isspace(c) && CTR_IS_NO_TOK(c) && code!=eofcode
	&& c != '+' && c!='*' && c!='/' && c!='=' && c!='>' && c!='<' && c!='&' //and c is not one of the special symbols
	) {
		buffer[i] = c; tokvlen++;
		i++;
		if (i > bflmt) {
			printf("[ERROR L001]: Token Buffer Exausted. Tokens may not exceed 100 bytes.");
			exit(1);
		}
		code++;
		c = *code;
	}

	return CTR_TOKEN_REF;
}


char* clex_readstr() {
	tokvlen=0;
	long memblock = 40;
	long page = 100; //100 byte pages
	char* strbuff = (char*) malloc(memblock);
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
		tokvlen ++;
		
		if (tokvlen > memblock) {
			memblock += page;
			beginbuff = (char*) realloc(beginbuff, memblock);
			if (beginbuff == NULL) {
				printf("Out of memory\n");
				exit(1);
			}
			//reset pointer, memory location might have been changed
			strbuff = beginbuff + (tokvlen -1);
		}
		
		escape = 0;
		*strbuff = c;
		strbuff++;
		code++;
		c = *code;
	}
	return beginbuff;
}
