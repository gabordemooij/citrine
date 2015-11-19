#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>

#include "citrine.h"


ctr_tnode* ctr_cparse_expr(int mode);
ctr_tnode* ctr_cparse_ret();

//precedence mode 0: no argument (allows processing of unary message, binary message and keyword message)
//precedence mode 1: as argument of keyword message (allows processing of unary message and binary message)
//precedence mode 2: as argument of binary message (only allows processing of unary message)
ctr_tnode* ctr_cparse_message(int mode) {
	long msgpartlen; //length of part of message string
	ctr_tnode* m = CTR_PARSER_CREATE_NODE();
	m->type = -1;
	int t = ctr_clex_tok();
	msgpartlen = ctr_clex_tok_value_length();
	if ((msgpartlen) > 255) {
		printf("Message too long\n");
		exit(1);
	}
	char* s = ctr_clex_tok_value();
	char* msg;
	msg = malloc(255*sizeof(char));
	memcpy(msg, s, msgpartlen);
	
	size_t ulen = ctr_getutf8len(msg, msgpartlen);
	int isBin = (ulen == 1);
	
	if (mode == 2 && isBin) {
		ctr_clex_putback();
		return m;
	}
	if (isBin) {
		if (ctr_mode_debug) printf("Parsing binary message: '%s' (mode: %d)\n", msg, mode);
		m->type = CTR_AST_NODE_BINMESSAGE;
		m->value = msg;
		m->vlen = msgpartlen;
		ctr_tlistitem* li = CTR_PARSER_CREATE_LISTITEM();
		if (ctr_mode_debug) printf("Binary argument start..\n");
		li->node = ctr_cparse_expr(2);
		if (ctr_mode_debug) printf("Binary argument end..\n");
		m->nodes = li;
		return m;
	}
	int lookAhead = ctr_clex_tok(); ctr_clex_putback();
	if (lookAhead == CTR_TOKEN_COLON) {
		if (mode > 0) {
			ctr_clex_putback();
			if (ctr_mode_debug) printf("> End of argument, next token: %s .\n", msg);
			return m;
		 }
		*(msg + msgpartlen) = ':';
		msgpartlen += 1;
		if ((msgpartlen) > 255) {
			printf("Message too long\n");
			exit(1);
		}
		if (ctr_mode_debug) printf("Message so far: %s\n", msg);
		m->type = CTR_AST_NODE_KWMESSAGE;
		t = ctr_clex_tok();
		int first = 1;
		ctr_tlistitem* li;
		ctr_tlistitem* curlistitem;
		while(1) {
			li = CTR_PARSER_CREATE_LISTITEM();
			if (ctr_mode_debug) printf("Next arg, message so far: %s \n", msg);
			li->node = ctr_cparse_expr(1);
			if (ctr_mode_debug) printf("Argument of keyword message has been parsed.\n");
			if (first) {
				m->nodes = li;
				curlistitem = m->nodes;
				first = 0;
			} else {
				curlistitem->next = li;
				curlistitem = li;
			}
			t = ctr_clex_tok();
			if (ctr_mode_debug) printf("Next token after argument = %d \n", t);
			if (t == CTR_TOKEN_DOT) break;
			if (t == CTR_TOKEN_FIN) break;
			if (t == CTR_TOKEN_CHAIN) break;
			if (t == CTR_TOKEN_PARCLOSE) break;
			if (t == CTR_TOKEN_REF) {
				long l = ctr_clex_tok_value_length(); 
				if ((msgpartlen + l) > 255) {
					printf("Message too long\n");
					exit(1);
				}
				memcpy( (msg+msgpartlen), ctr_clex_tok_value(), l);
				msgpartlen = msgpartlen + l;
				*(msg + msgpartlen) = ':';
				msgpartlen ++;
				t = ctr_clex_tok();
				if (t != CTR_TOKEN_COLON) {
					printf("Expected colon. %s \n",msg);
					exit(1);
				}
			}
		}
		if (ctr_mode_debug) printf("Putting back.\n");
		ctr_clex_putback(); //not a colon so put back
		if (ctr_mode_debug) printf("Parsing keyword message: '%s' (mode: %d) \n", msg, mode);
		m->value = msg;
		m->vlen = msgpartlen;
	} else {
		m->type = CTR_AST_NODE_UNAMESSAGE;
		m->value = msg;
		m->vlen = msgpartlen;
		if (ctr_mode_debug) printf("Parsing unary message: '%s' (mode: %d) token = %d \n", msg, mode, lookAhead);
	}
	return m;
}

ctr_tlistitem* ctr_cparse_messages(ctr_tnode* r, int mode) {
	if (ctr_mode_debug) printf("Parsing messages.\n");
	int t = ctr_clex_tok();
	ctr_tlistitem* pli;
	ctr_tlistitem* li;
	ctr_tlistitem* fli;
	int first = 1;
	ctr_tnode* node;
	//explicit chaining (,) only allowed for keyword message: Console write: 3 factorial, write: 3 factorial is not possible otherwise. 
	while ((t == CTR_TOKEN_REF || (t == CTR_TOKEN_CHAIN && node && node->type == CTR_AST_NODE_KWMESSAGE))) {
		if (t == CTR_TOKEN_CHAIN) {
			t = ctr_clex_tok();
			if (t != CTR_TOKEN_REF) {
				printf("Expected message.\n");
				exit(1);
			}
		}
		li = CTR_PARSER_CREATE_LISTITEM();
		if (ctr_mode_debug) printf("Next message...\n");
		ctr_clex_putback();
		node = ctr_cparse_message(mode);
		if (node->type == -1) {
			if (ctr_mode_debug) printf("Ending message sequence.\n");
			if (first) {
				if (ctr_mode_debug) printf("First so return NULL.\n");
				return NULL;
			}
			ctr_clex_tok();
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
		t = ctr_clex_tok();
		if (ctr_mode_debug) printf("Next token in message line is: %d \n",t);
	}
	if (ctr_mode_debug) printf("Putting back token... \n");
	ctr_clex_putback();
	return fli;
}

ctr_tnode* ctr_cparse_popen() {
	ctr_clex_tok();
	if (ctr_mode_debug) printf("Parsing paren open (.\n");
	ctr_tnode* r = CTR_PARSER_CREATE_NODE();
	r->type = CTR_AST_NODE_NESTED;
	ctr_tlistitem* li = CTR_PARSER_CREATE_LISTITEM();
	r->nodes = li;
	li->node = ctr_cparse_expr(0);
	int t = ctr_clex_tok();
	if (t != CTR_TOKEN_PARCLOSE) {
		printf("Error, expected ). \n");
		exit(1);
	}
	return r;
}

ctr_tnode* ctr_cparse_block() {
	if (ctr_mode_debug) printf("Parsing code block.\n");
	ctr_clex_tok();
	ctr_tnode* r = CTR_PARSER_CREATE_NODE();
	r->type = CTR_AST_NODE_CODEBLOCK;
	ctr_tlistitem* codeBlockPart1 = CTR_PARSER_CREATE_LISTITEM();
	r->nodes = codeBlockPart1;
	ctr_tlistitem* codeBlockPart2 = CTR_PARSER_CREATE_LISTITEM();
	r->nodes->next = codeBlockPart2;
	ctr_tnode* paramList = CTR_PARSER_CREATE_NODE();
	ctr_tnode* codeList  = CTR_PARSER_CREATE_NODE();
	codeBlockPart1->node = paramList;
	codeBlockPart2->node = codeList;
	paramList->type = CTR_AST_NODE_PARAMLIST;
	codeList->type = CTR_AST_NODE_INSTRLIST;
	int t = ctr_clex_tok();
	int first = 1;
	ctr_tlistitem* previousListItem;
	while(t == CTR_TOKEN_REF) {
		ctr_tlistitem* paramListItem = CTR_PARSER_CREATE_LISTITEM();
		ctr_tnode* paramItem = CTR_PARSER_CREATE_NODE();
		long l = ctr_clex_tok_value_length();
		paramItem->value = malloc(sizeof(char) * l);
		memcpy(paramItem->value, ctr_clex_tok_value(), l);
		paramItem->vlen = l;
		paramListItem->node = paramItem;
		if (first) {
			paramList->nodes = paramListItem;
			previousListItem = paramListItem;
			first = 0;
		} else {
			previousListItem->next = paramListItem;
			previousListItem = paramListItem;
		}
		t = ctr_clex_tok();
	}
	if (t != CTR_TOKEN_BLOCKPIPE) {
		printf("Error expected blockpipe.");
		exit(1);
	}
	t = ctr_clex_tok();
	first = 1;
	ctr_tlistitem* previousCodeListItem;
	while((first || t == CTR_TOKEN_DOT)) {
		if (first) {
			if (ctr_mode_debug) printf("First, so put back\n");
			ctr_clex_putback();
		}
		t = ctr_clex_tok();
		if (t == CTR_TOKEN_BLOCKCLOSE) break;
		ctr_clex_putback();
		ctr_tlistitem* codeListItem = CTR_PARSER_CREATE_LISTITEM();
		ctr_tnode* codeNode = CTR_PARSER_CREATE_NODE();
		if (ctr_mode_debug) printf("--------> %d %s \n", t, ctr_clex_tok_value());
		if (t == CTR_TOKEN_RET) {
			codeNode = ctr_cparse_ret();
		} else {
			codeNode = ctr_cparse_expr(0);
		}
		codeListItem->node = codeNode;
		if (first) {
			codeList->nodes = codeListItem;
			previousCodeListItem = codeListItem;
			first = 0;
		} else {
			previousCodeListItem->next = codeListItem;
			previousCodeListItem = codeListItem;
		}
		t = ctr_clex_tok();
		if (t != CTR_TOKEN_DOT) {
			printf("Expected . but got: %d.\n", t);
			exit(1);
		}
	}
	return r;
}

ctr_tnode* ctr_cparse_ref() {
	ctr_clex_tok();
	ctr_tnode* r = CTR_PARSER_CREATE_NODE();
	r->type = CTR_AST_NODE_REFERENCE;
	r->vlen = ctr_clex_tok_value_length();
	char* tmp = ctr_clex_tok_value();
	if (strncmp("my", tmp, 2)==0 && r->vlen == 2) {
		int t = ctr_clex_tok();
		if (t != CTR_TOKEN_REF) {
			printf("'My' should always be followed by property name!\n");
			exit(1);
		}
		tmp = ctr_clex_tok_value();
		r->modifier = 1;
		r->vlen = ctr_clex_tok_value_length();
	}
	r->value = malloc(r->vlen);
	memcpy(r->value, tmp, r->vlen);
	return r;
}

ctr_tnode* ctr_cparse_string() {
	if (ctr_mode_debug) printf("Parsing STRING. \n");
	ctr_clex_tok();
	ctr_tnode* r = CTR_PARSER_CREATE_NODE();
	r->type = CTR_AST_NODE_LTRSTRING;
	char* n = ctr_clex_readstr();
	long vlen = ctr_clex_tok_value_length();
	r->value = calloc(sizeof(char), vlen);
	memcpy(r->value, n, vlen);
	r->vlen = vlen;
	ctr_clex_tok(); //eat trailing quote.
	return r;
}


ctr_tnode* ctr_cparse_number() {
	ctr_clex_tok();
	ctr_tnode* r = CTR_PARSER_CREATE_NODE();
	r->type = CTR_AST_NODE_LTRNUM;
	char* n = ctr_clex_tok_value();
	long l = ctr_clex_tok_value_length();
	r->value = calloc(sizeof(char), l);
	memcpy(r->value, n, l);
	r->vlen = l;
	if (ctr_mode_debug) printf("Parsing number: %s.\n", r->value);
	return r;
}

ctr_tnode* ctr_cparse_false() {
	ctr_clex_tok();
	ctr_tnode* r = CTR_PARSER_CREATE_NODE();
	r->type = CTR_AST_NODE_LTRBOOLFALSE;
	ASSIGN_STRING(r, value, "False", 5);
	r->vlen = 5;
	return r;
}

ctr_tnode* ctr_cparse_true() {
	ctr_clex_tok();
	ctr_tnode* r = CTR_PARSER_CREATE_NODE();
	r->type = CTR_AST_NODE_LTRBOOLTRUE;
	ASSIGN_STRING(r, value,"True",4);	
	r->vlen = 4;
	return r;
}

ctr_tnode* ctr_cparse_nil() {
	ctr_clex_tok();
	ctr_tnode* r = CTR_PARSER_CREATE_NODE();
	r->type = CTR_AST_NODE_LTRNIL;
	r->value = "Nil";
	r->vlen = 3;
	return r;
}

ctr_tnode* ctr_cparse_receiver() {
	if (ctr_mode_debug) printf("Parsing receiver.\n");
	int t = ctr_clex_tok();
	ctr_clex_putback();
	if (t == CTR_TOKEN_NIL) return ctr_cparse_nil();
	if (t == CTR_TOKEN_BOOLEANYES) return ctr_cparse_true();
	if (t == CTR_TOKEN_BOOLEANNO) return ctr_cparse_false();
	if (t == CTR_TOKEN_NUMBER) return ctr_cparse_number();
	if (t == CTR_TOKEN_QUOTE) return ctr_cparse_string();
	if (t == CTR_TOKEN_REF) return ctr_cparse_ref();
	if (t == CTR_TOKEN_BLOCKOPEN) return ctr_cparse_block();
	if (t == CTR_TOKEN_PAROPEN) return ctr_cparse_popen();
	printf("Error, unexpected token: %d.\n", t);
	exit(1);
}

ctr_tnode* ctr_cparse_assignment(ctr_tnode* r) {
	ctr_clex_tok();
	ctr_tnode* a = CTR_PARSER_CREATE_NODE();
	ctr_tnode* assignmentExpr = CTR_PARSER_CREATE_NODE();
	ctr_tlistitem* li = CTR_PARSER_CREATE_LISTITEM();
	ctr_tlistitem* liAssignExpr = CTR_PARSER_CREATE_LISTITEM();
	a->type = CTR_AST_NODE_EXPRASSIGNMENT;
	a->nodes = li;
	li->node = r;
	assignmentExpr = ctr_cparse_expr(0);
	liAssignExpr->node = assignmentExpr;
	li->next = liAssignExpr;
	return a;
}

ctr_tnode* ctr_cparse_expr(int mode) {
	if (ctr_mode_debug) printf("Parsing expression (mode: %d).\n", mode);	
	ctr_tnode* r = ctr_cparse_receiver();
	ctr_tnode* e;
	int t2 = ctr_clex_tok();
	if (ctr_mode_debug) printf("First token after receiver = %d \n", t2);
	ctr_clex_putback();
	if (r->type == CTR_AST_NODE_REFERENCE && t2 == CTR_TOKEN_ASSIGNMENT) {
		e = ctr_cparse_assignment(r);
	} else if (t2 != CTR_TOKEN_DOT && t2 != CTR_TOKEN_PARCLOSE && t2 != CTR_TOKEN_CHAIN) {
		e = CTR_PARSER_CREATE_NODE();
		e->type = CTR_AST_NODE_EXPRMESSAGE;
		ctr_tlistitem* nodes = ctr_cparse_messages(r, mode);
		if (nodes == NULL) {
			int t = ctr_clex_tok();
			ctr_clex_putback();
			if (ctr_mode_debug) printf("No messages, return. Next token: %d.\n", t);
			return r; //no messages, then just return receiver (might be in case of argument).
		}
		ctr_tlistitem* rli = CTR_PARSER_CREATE_LISTITEM();
		rli->node = r;
		rli->next = nodes;
		e->nodes = rli;
	} else {
		if (ctr_mode_debug) printf("Returning receiver. \n");
		return r;
	}
	return e;
}


ctr_tnode* ctr_cparse_ret() {
	ctr_clex_tok();
	ctr_tnode* r = CTR_PARSER_CREATE_NODE();
	r->type = CTR_AST_NODE_RETURNFROMBLOCK;
	ctr_tlistitem* li = CTR_PARSER_CREATE_LISTITEM();
	r->nodes = li;
	li->node = ctr_cparse_expr(0);
	return r;
}

ctr_tnode* ctr_cparse_fin() {
	ctr_clex_tok();
	ctr_tnode* f = CTR_PARSER_CREATE_NODE();
	f->type = CTR_AST_NODE_ENDOFPROGRAM;
	return f;
}

ctr_tlistitem* ctr_cparse_statement() {
	ctr_tlistitem* li = CTR_PARSER_CREATE_LISTITEM();
	int t = ctr_clex_tok();
	if (ctr_mode_debug) printf("Parsing next statement of program, token = %d (%s).\n", t, ctr_clex_tok_value());
	ctr_clex_putback();
	if (t == CTR_TOKEN_FIN) {
		li->node = ctr_cparse_fin();
		return li;
	} else if (t == CTR_TOKEN_RET) {
		li->node = ctr_cparse_ret();
	} else {
		li->node = ctr_cparse_expr(0);
	}
	t = ctr_clex_tok();
	if (t != CTR_TOKEN_DOT) {
		printf("Expected . but got: %d.\n", t);
		exit(1);
	}
	return li;
}

ctr_tnode* ctr_cparse_program() {
	ctr_tnode* program = CTR_PARSER_CREATE_NODE();
	ctr_tlistitem* pli;
	int first = 1;
	while(1) {
		ctr_tlistitem* li = ctr_cparse_statement();
		if (first) {
			first = 0;
			program->nodes = li;
		} else {
			pli->next = li;
		}
		if (li->node->type == CTR_AST_NODE_ENDOFPROGRAM) break;
		pli = li;
	}
	return program;
}

ctr_tnode*  dparse_parse(char* prg) {
	ctr_clex_load(prg);
	ctr_tnode* program = ctr_cparse_program();
	return program;
}
