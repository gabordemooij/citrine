#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>

#include "uthash.h"
#include "citrine.h"


tnode* cparse_expr(int mode);
tnode* cparse_ret();

//precedence mode 0: no argument (allows processing of unary message, binary message and keyword message)
//precedence mode 1: as argument of keyword message (allows processing of unary message and binary message)
//precedence mode 2: as argument of binary message (only allows processing of unary message)
tnode* cparse_message(int mode) {
	tnode* m = CTR_PARSER_CREATE_NODE();
	m->type = -1;
	int t = clex_tok();
	char* s = clex_tok_value();
	char* msg;
	msg = calloc(sizeof(char), 255);
	strcat(msg, s);
	int isBin = (strlen(msg)==2 && (strcmp("&&",msg)==0 || strcmp("||",msg)==0 || strcmp("==",msg)==0 || strcmp("!=",msg)==0 || strcmp(">=",msg)==0 || strcmp("<=",msg)==0));
	isBin = (isBin || (strlen(msg)==1 && (strcmp(">",msg)==0 || strcmp("<",msg)==0 || strcmp("*",msg)==0 || strcmp("/",msg)==0 || strcmp("+",msg)==0 || strcmp("-",msg)==0)));
	if (mode == 2 && isBin) {
		clex_putback();
		return m;
	}
	if (isBin) {
		if (debug) printf("Parsing binary message: '%s' (mode: %d)\n", msg, mode);
		m->type = BINMESSAGE;
		m->value = msg;
		tlistitem* li = CTR_PARSER_CREATE_LISTITEM();
		if (debug) printf("Binary argument start..\n");
		li->node = cparse_expr(2);
		if (debug) printf("Binary argument end..\n");
		m->nodes = li;
		return m;
	}
	int lookAhead = clex_tok(); clex_putback();
	if (lookAhead == COLON) {
		if (mode > 0) {
			clex_putback();
			if (debug) printf("> End of argument, next token: %s .\n", msg);
			return m;
		 }
		strcat(msg,":");
		if (debug) printf("Message so far: %s\n", msg);
		m->type = KWMESSAGE;
		t = clex_tok();
		int antiCrash = 0;
		int first = 1;
		tlistitem* li;
		tlistitem* curlistitem;
		while((antiCrash++)<100) {
			li = CTR_PARSER_CREATE_LISTITEM();
			if (debug) printf("Next arg, message so far: %s \n", msg);
			li->node = cparse_expr(1);
			if (debug) printf("Argument of keyword message has been parsed.\n");
			if (first) {
				m->nodes = li;
				curlistitem = m->nodes;
				first = 0;
			} else {
				curlistitem->next = li;
				curlistitem = li;
			}
			t = clex_tok();
			if (debug) printf("Next token after argument = %d \n", t);
			if (t == DOT) break;
			if (t == FIN) break;
			if (t == CHAIN) break;
			if (t == PARCLOSE) break;
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
		if (debug) printf("Putting back.\n");
		clex_putback(); //not a colon so put back
		if (debug) printf("Parsing keyword message: '%s' (mode: %d) \n", msg, mode);
		m->value = msg;
	} else {
		m->type = UNAMESSAGE;
		m->value = msg;
		if (debug) printf("Parsing unary message: '%s' (mode: %d) token = %d \n", msg, mode, lookAhead);
	}
	return m;
}

tlistitem* cparse_messages(tnode* r, int mode) {
	if (debug) printf("Parsing messages.\n");
	int t = clex_tok();
	
	int antiCrash = 0;
	tlistitem* pli;
	tlistitem* li;
	tlistitem* fli;
	int first = 1;
	tnode* node;
	//explicit chaining (,) only allowed for keyword message: Console write: 3 factorial, write: 3 factorial is not possible otherwise. 
	while ((antiCrash++<100) && (t == REF || (t == CHAIN && node && node->type == KWMESSAGE))) {
		if (t == CHAIN) {
			t = clex_tok();
			if (t != REF) {
				printf("Expected message.\n");
				exit(1);
			}
		}
		li = CTR_PARSER_CREATE_LISTITEM();
		if (debug) printf("Next message...\n");
		clex_putback();
		node = cparse_message(mode);
		if (node->type == -1) {
			if (debug) printf("Ending message sequence.\n");
			if (first) {
				if (debug) printf("First so return NULL.\n");
				return NULL;
			}
			clex_tok();
			break;
		}
		li->node = node;
		if (first) {
			first = 0;
			pli = li;
			fli = li;
		} else {
			pli->next = li;
			pli = li;
		}
		t = clex_tok();
		if (debug) printf("Next token in message line is: %d \n",t);
	}
	if (debug) printf("Putting back token... \n");
	clex_putback();
	return fli;
}

tnode* cparse_popen() {
	clex_tok();
	if (debug) printf("Parsing paren open (.\n");
	tnode* r = CTR_PARSER_CREATE_NODE();
	r->type = NESTED;
	tlistitem* li = CTR_PARSER_CREATE_LISTITEM();
	r->nodes = li;
	li->node = cparse_expr(0);
	int t = clex_tok();
	if (t != PARCLOSE) {
		printf("Error, expected ). \n");
		exit(1);
	}
	return r;
}

tnode* cparse_block() {
	if (debug) printf("Parsing code block.\n");
	clex_tok();
	tnode* r = CTR_PARSER_CREATE_NODE();
	r->type = CODEBLOCK;
	tlistitem* codeBlockPart1 = CTR_PARSER_CREATE_LISTITEM();
	r->nodes = codeBlockPart1;
	tlistitem* codeBlockPart2 = CTR_PARSER_CREATE_LISTITEM();
	r->nodes->next = codeBlockPart2;
	tnode* paramList = CTR_PARSER_CREATE_NODE();
	tnode* codeList  = CTR_PARSER_CREATE_NODE();
	codeBlockPart1->node = paramList;
	codeBlockPart2->node = codeList;
	paramList->type = PARAMLIST;
	codeList->type = INSTRLIST;
	int t = clex_tok();
	int antiCrash2 = 0;
	int first = 1;
	tlistitem* previousListItem;
	while((antiCrash2++ < 10) && t == REF) {
		tlistitem* paramListItem = CTR_PARSER_CREATE_LISTITEM();
		tnode* paramItem = CTR_PARSER_CREATE_NODE();
		CTR_PARSER_GET_TOKVAL(paramItem);
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
	while((antiCrash++ < 100) && (first || t == DOT)) {
		if (first) {
			if (debug) printf("First, so put back\n");
			clex_putback();
		}
		t = clex_tok();
		if (t == BLOCKCLOSE) break;
		clex_putback();
		tlistitem* codeListItem = CTR_PARSER_CREATE_LISTITEM();
		tnode* codeNode = CTR_PARSER_CREATE_NODE();
		if (debug) printf("--------> %d %s \n", t, clex_tok_value());
		if (t == RET) {
			codeNode = cparse_ret();
		} else {
			codeNode = cparse_expr(0);
		}
		if (debug) printf("<-------- \n");
		
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
		if (t != DOT) {
			printf("Expected . but got: %d.\n", t);
			exit(1);
		}
	}
	return r;
}

tnode* cparse_ref() {
	clex_tok();
	tnode* r = CTR_PARSER_CREATE_NODE();
	r->type = REFERENCE;
	char* tmp = clex_tok_value();
	if (strcmp("my", tmp)==0) {
		int t = clex_tok();
		if (t != REF) {
			printf("'My' should always be followed by property name!\n");
			exit(1);
		}
		tmp = clex_tok_value();
		r->modifier = 1;
	}
	r->value = malloc(strlen(tmp));
	strcpy(r->value, tmp);
	return r;
}

tnode* cparse_string() {
	if (debug) printf("Parsing STRING. \n");
	clex_tok();
	tnode* r = CTR_PARSER_CREATE_NODE();
	r->type = LTRSTRING;
	char* n = clex_readstr();
	long vlen = clex_len;
	r->value = calloc(sizeof(char), vlen);
	strncpy(r->value, n, vlen);
	r->vlen = vlen;
	clex_tok(); //eat trailing quote.
	return r;
}


tnode* cparse_number() {
	clex_tok();
	tnode* r = CTR_PARSER_CREATE_NODE();
	r->type = LTRNUM;
	char* n = clex_tok_value();
	r->value = calloc(sizeof(char), strlen(n));
	strcpy(r->value, n);
	r->vlen = strlen(n);
	if (debug) printf("Parsing number: %s.\n", r->value);
	return r;
}

tnode* cparse_false() {
	clex_tok();
	tnode* r = CTR_PARSER_CREATE_NODE();
	r->type = LTRBOOLFALSE;
	ASSIGN_STRING(r, value, "False", 5);
	r->vlen = 5;
	return r;
}

tnode* cparse_true() {
	clex_tok();
	tnode* r = CTR_PARSER_CREATE_NODE();
	r->type = LTRBOOLTRUE;
	ASSIGN_STRING(r, value,"True",4);
	r->vlen = 4;
	return r;
}

tnode* cparse_nil() {
	clex_tok();
	tnode* r = CTR_PARSER_CREATE_NODE();
	r->type = LTRNIL;
	r->value = "Nil";
	return r;
}

tnode* cparse_receiver() {
	if (debug) printf("Parsing receiver.\n");
	int t = clex_tok();
	clex_putback();
	if (t == NIL) return cparse_nil();
	if (t == BOOLEANYES) return cparse_true();
	if (t == BOOLEANNO) return cparse_false();
	if (t == NUMBER) return cparse_number();
	if (t == QUOTE) return cparse_string();
	if (t == REF) return cparse_ref();
	if (t == BLOCKOPEN) return cparse_block();
	if (t == PAROPEN) return cparse_popen();
	printf("Error, unexpected token: %d.\n", t);
	exit(1);
}

tnode* cparse_assignment(tnode* r) {
	clex_tok();
	tnode* a = CTR_PARSER_CREATE_NODE();
	tnode* assignmentExpr = CTR_PARSER_CREATE_NODE();
	tlistitem* li = CTR_PARSER_CREATE_LISTITEM();
	tlistitem* liAssignExpr = CTR_PARSER_CREATE_LISTITEM();
	a->type = EXPRASSIGNMENT;
	a->nodes = li;
	li->node = r;
	assignmentExpr = cparse_expr(0);
	liAssignExpr->node = assignmentExpr;
	li->next = liAssignExpr;
	return a;
}

tnode* cparse_expr(int mode) {
	if (debug) printf("Parsing expression (mode: %d).\n", mode);	
	tnode* r = cparse_receiver();
	tnode* e;
	int t2 = clex_tok();
	if (debug) printf("First token after receiver = %d \n", t2);
	clex_putback();
	if (r->type == REFERENCE && t2 == ASSIGNMENT) {
		e = cparse_assignment(r);
	} else if (t2 != DOT && t2 != PARCLOSE && t2 != CHAIN) {
		e = CTR_PARSER_CREATE_NODE();
		e->type = EXPRMESSAGE;
		tlistitem* nodes = cparse_messages(r, mode);
		if (nodes == NULL) {
			int t = clex_tok();
			clex_putback();
			if (debug) printf("No messages, return. Next token: %d.\n", t);
			return r; //no messages, then just return receiver (might be in case of argument).
		}
		tlistitem* rli = CTR_PARSER_CREATE_LISTITEM();
		rli->node = r;
		rli->next = nodes;
		e->nodes = rli;
	} else {
		if (debug) printf("Returning receiver. \n");
		return r;
	}
	return e;
}


tnode* cparse_ret() {
	clex_tok();
	tnode* r = CTR_PARSER_CREATE_NODE();
	r->type = RETURNFROMBLOCK;
	tlistitem* li = CTR_PARSER_CREATE_LISTITEM();
	r->nodes = li;
	li->node = cparse_expr(0);
	return r;
}

tnode* cparse_fin() {
	clex_tok();
	tnode* f = CTR_PARSER_CREATE_NODE();
	f->type = ENDOFPROGRAM;
	return f;
}

tlistitem* cparse_statement() {
	tlistitem* li = CTR_PARSER_CREATE_LISTITEM();
	int t = clex_tok();
	if (debug) printf("Parsing next statement of program, token = %d (%s).\n", t, clex_tok_value());
	clex_putback();
	if (t == FIN) {
		li->node = cparse_fin();
		return li;
	} else if (t == RET) {
		li->node = cparse_ret();
	} else {
		li->node = cparse_expr(0);
	}
	t = clex_tok();
	if (t != DOT) {
		printf("Expected . but got: %d.\n", t);
		exit(1);
	}
	return li;
}

tnode* cparse_program() {
	tnode* program = CTR_PARSER_CREATE_NODE();
	int antiCrash = 0;
	tlistitem* pli;
	int first = 1;
	while((antiCrash++)<100) {
		tlistitem* li = cparse_statement();
		if (first) {
			first = 0;
			program->nodes = li;
		} else {
			pli->next = li;
		}
		if (li->node->type == ENDOFPROGRAM) break;
		pli = li;
	}
	return program;
}

tnode*  dparse_parse(char* prg) {
	clex_load(prg);
	tnode* program = cparse_program();
	return program;
}
