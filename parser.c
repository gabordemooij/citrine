#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>

#include "lexer.h"
#include "parser.h"

void tree(tnode* node, int indent);

tnode* dparse_expr();
tnode* dparse_message() {
	int t;
	char* msg = (char*) calloc(sizeof(char*), 255);
	tnode* m = N();
	strcat(msg, clex_tok_value());
	
	
	if (
		
		(
			strlen(msg)==2 && (
				strcmp("==",msg)==0 ||
				strcmp(">=",msg)==0 ||
				strcmp("<=",msg)==0
			)
		)
		
		||	
		
		(
			strlen(msg)==1 && (
				strcmp(">",msg)==0  ||
				strcmp("<",msg)==0  ||
				strcmp("*",msg)==0  ||
				strcmp("/",msg)==0  ||
				strcmp("+",msg)==0  ||
				strcmp("-",msg)==0
			)
		)
	) {
		m->type = BINMESSAGE;
		m->value = msg;
		tlistitem* li = LI();
		li->node = dparse_expr(0);
		m->nodes = li;
		return m;
	}
	int lookAhead = clex_tok();
	clex_putback();
	if (lookAhead == COLON) {
		strcat(msg,":");
		m->type = KWMESSAGE;
		t = clex_tok();
		int antiCrash = 0;
		int first = 1;
		tlistitem* li;
		tlistitem* curlistitem;
		while((antiCrash++)<100) {
			li = LI();
			li->node = dparse_expr(1);
			if (first) {
				m->nodes = li;
				curlistitem = m->nodes;
				first = 0;
			} else {
				curlistitem->next = li;
				curlistitem = li;
			}
			t = clex_tok();
			
			if (t == DOT) break;
			if (t == FIN) break;
			if (t == CHAIN) break;
			if (t == REF) {
				//@todo memory management... overflow possible!
				strcat(msg, clex_tok_value());
				strcat(msg, ":");
				t = clex_tok();
				if (t != COLON) {
					printf("Expected colon. %s \n",msg);
					exit(1);
				}
			}
		}
		clex_putback(); //not a colon so put back
		strcat(msg, "\0");
		m->value = msg;
	} else {
		m->type = UNAMESSAGE;
		m->value = msg;
	}
	return m;
}


tlistitem* dparse_messages() {
	int t = clex_tok();
	int antiCrash = 0;
	tlistitem* pli;
	tlistitem* li;
	tlistitem* fli;
	int first = 1;
	while ((antiCrash++<100) && (t == REF || t == CHAIN)) {
		if (t == CHAIN) {
			t = clex_tok();
			if (t != REF) printf("Expected message.\n");
		}
		li = LI();
		li->node = dparse_message();
		if (first) {
			first = 0;
			pli = li;
			fli = li;
		} else {
			pli->next = li;
			pli = li;
		}
		t = clex_tok();
	}
	clex_putback();
	return fli;
}

tnode* dparse_expr(int argumentMode) {
	tnode* e = N();
	tnode* r = N();
	int t = clex_tok();
	
	if (t == PAROPEN) {
		r->type = NESTED;
		tlistitem* li = LI();
		r->nodes = li;
		li->node = dparse_expr(0);
		t = clex_tok();
		if (t != PARCLOSE) {
			printf("Error, expected ).\n");
			exit(1);
		}
		t = clex_tok();
		if (t != REF) {
			clex_putback(); //dont eat the dot...
			return r;
		}
		if (t == REF) {
			e->type = EXPRMESSAGE;
			clex_putback();
			tlistitem* li = LI();
			li->node = r;
			e->nodes = li;
			tlistitem* fli = dparse_messages();
			li->next = fli;
		}
		return e;
	}
	
	if (t == RET) {
		e->type = RETURNFROMBLOCK;
		tlistitem* returnListItem = LI();
		tnode* returnExpr = N();
		returnExpr = dparse_expr(0);
		returnListItem->node = returnExpr;
		e->nodes = returnListItem;
		return e;
	}
	
	if (t == BLOCKOPEN) {
		e->type = CODEBLOCK;
		tlistitem* codeBlockPart1 = LI();
		e->nodes = codeBlockPart1;
		tlistitem* codeBlockPart2 = LI();
		e->nodes->next = codeBlockPart2;
		tnode* paramList = N();
		tnode* codeList  = N();
		codeBlockPart1->node = paramList;
		codeBlockPart2->node = codeList;
		paramList->type = PARAMLIST;
		codeList->type = INSTRLIST;
		t = clex_tok();
		int antiCrash2 = 0;
		int first = 1;
		tlistitem* previousListItem;
		while((antiCrash2++ < 10) && t == REF) {
			tlistitem* paramListItem = LI();
			tnode* paramItem = N();
			TVAL(paramItem);
			paramListItem->node = paramItem;
			if (first) {
				paramList->nodes = paramListItem;
				previousListItem = paramListItem;
				first = 0;
			} else {
				previousListItem->next = paramListItem;
				previousListItem = paramListItem;
			}
			t = clex_tok();
		}
		if (t != BLOCKPIPE) {
			printf("Error expected blockpipe.");
			exit(1);
		}
		t = clex_tok();
		int antiCrash = 0;
		first = 1;
		tlistitem* previousCodeListItem;
		while((antiCrash++ < 10) && (first || t == DOT)) {
			if (first) clex_putback();
			
			t = clex_tok();
			if (t == BLOCKCLOSE) break;
			clex_putback();
			
			tlistitem* codeListItem = LI();
			tnode* codeNode = N();
			codeNode = dparse_expr(0);
			codeListItem->node = codeNode;
			if (first) {
				codeList->nodes = codeListItem;
				previousCodeListItem = codeListItem;
				first = 0;
			} else {
				previousCodeListItem->next = codeListItem;
				previousCodeListItem = codeListItem;
			}
			t = clex_tok();
		}
		return e;
	}
	
	if (t == NIL) {
		r->type = LTRNIL;
		r->value = "Nil";
		if (argumentMode == 2) {
			return r;
		}
		t = clex_tok();
		if (t == PARCLOSE || t == DOT) {
			clex_putback();//put back the dot, it's for the program
			return r;
		}
		if (t == REF) {
			e->type = EXPRMESSAGE;
			clex_putback();
			tlistitem* li = LI();
			li->node = r;
			e->nodes = li;
			tlistitem* fli = dparse_messages();
			li->next = fli;
		}
	}
	
	if (t == BOOLEANYES || t == BOOLEANNO) {
		char* str;
		if (t == BOOLEANYES) {
			r->type = LTRBOOLTRUE;
			str = "True";
		} else {
			r->type = LTRBOOLFALSE;
			str = "False";
		}
		r->value = str;
		if (argumentMode == 2) {
			return r;
		}
		t = clex_tok();
		if (t == PARCLOSE || t == DOT) {
			clex_putback();//put back the dot, it's for the program
			return r;
		}
		if (t == REF) {
			e->type = EXPRMESSAGE;
			clex_putback();
			tlistitem* li = LI();
			li->node = r;
			e->nodes = li;
			tlistitem* fli = dparse_messages();
			li->next = fli;
		}
		return e;
	}
	
	if (t == NUMBER) {
		r->type = LTRNUM;
		char* tmp = clex_tok_value();
		r->value = calloc(sizeof(char), strlen(tmp));
		strcpy(r->value, tmp);
		if (argumentMode == 2) {
			return r;
		}
		t = clex_tok();
		if (t == PARCLOSE || t == DOT) {
			clex_putback();//put back the dot, it's for the program
			return r;
		}
		if (t == REF) {
			e->type = EXPRMESSAGE;
			clex_putback();
			tlistitem* li = LI();
			li->node = r;
			e->nodes = li;
			tlistitem* fli = dparse_messages();
			li->next = fli;
		}
		return e;
	}
	
	if (t == QUOTE) {
		e->type = LTRSTRING;
		e->value = clex_readstr();
		clex_tok(); //get rid of closing quote!
	}
	
	if (t == REF) {
		r->type = REFERENCE;
		char* tmp = clex_tok_value();
		if (strcmp("my", tmp)==0) {
			t = clex_tok();
			if (t != REF) {
				printf("'My' should always be followed by property name!\n");
				exit(1);
			}
			tmp = clex_tok_value();
			r->modifier = 1;
		}
		r->value = malloc(strlen(tmp));
		strcpy(r->value, tmp);
		t = clex_tok();
		if (t == DOT || t == PARCLOSE || argumentMode) {
			r->type = REFERENCE;
			clex_putback();
			return r;
		}
		tlistitem* li = LI();
		li->node = r;
		e->nodes = li;
		if (t == ASSIGNMENT) {
			e->type = EXPRASSIGNMENT;
			tnode* assignmentExpr = N();
			assignmentExpr = dparse_expr(0);
			tlistitem* liAssignExpr = LI();
			liAssignExpr->node = assignmentExpr;
			li->next = liAssignExpr;
		} else if (t == REF)  {		
			e->type = EXPRMESSAGE;
			clex_putback();
			tlistitem* li = LI();
			li->node = r;
			e->nodes = li;
			tlistitem* fli = dparse_messages();
			li->next = fli;
		} else {
			printf("Error expected assignment or message \n");
			exit(1);
		}
	}
	return e;
}

tnode* dparse_program() {
	tnode* program = N();
	int t = clex_tok();
	int antiCrash = 0;
	tlistitem* li;
	tlistitem* pli;
	int first = 1;
	while((antiCrash++)<100 && (first || t == DOT)) {
		if (first) clex_putback(); //if first (not dot) put back token
		tnode* e = N();
		t = clex_tok();
		if (t == FIN) break; //in case there is a trailing dot..
		clex_putback();
		e = dparse_expr(0);
		li = LI();
		li->node = e;
		if (first) {
			program->nodes = li;
			pli = li;
			first = 0;		
		} else {
			pli->next = li;
			pli = li;
		}
		t = clex_tok();
		if (t!=DOT && t!=FIN) {
			printf("Expected . but got: %d \n", t);
			exit(1);
		}
	}
	//the FIN node to end the program.
	tlistitem* finli = LI();
	li->next = finli;
	tnode* fin = N();
	fin->type = ENDOFPROGRAM;
	finli->node = fin;
	return program;
}

tnode*  dparse_parse(char* prg) {
	clex_load(prg);
	tnode* program = dparse_program();
	return program;
}
