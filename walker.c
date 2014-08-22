#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>

#include "uthash.h"
#include "object.h"

#include "parser.h"
#include "tools.h"

int debug;

obj* dwlk_expr(tnode* node);

obj* dwlk_return(tnode* node) {
	if (!node->nodes) {
		printf("Invalid return expression.\n");
		exit(1);
	}
	tlistitem* li = node->nodes;
	if (!li->node) {
		printf("Invalid return expression 2.\n");
		exit(1);
	} 
	obj* e = O();
	e =  dwlk_expr(li->node);
	return e;
}

obj* dwlk_message(tnode* paramNode) {
	obj* result;
	int antiCrash = 0;
	tlistitem* eitem = paramNode->nodes;
	tnode* receiverNode = eitem->node;
	tnode* msgnode;
	tlistitem* li = eitem;
	char* message;
	tlistitem* argumentList;
	obj* r;
	if (receiverNode->type == REFERENCE) {
		r = dnk_find(receiverNode->value);
		if (!r) {
			printf("Object not found: %s \n", receiverNode->value);
			exit(1);
		}
	} else if (receiverNode->type == LTRNIL ) {
		r = dnk_build_nil();
	} else if (receiverNode->type == LTRBOOLTRUE ) {
		r = dnk_build_bool(1);
	} else if (receiverNode->type == LTRBOOLFALSE ) {
		r = dnk_build_bool(0);
	} else if (receiverNode->type == LTRSTRING ) {
		r = dnk_build_string(receiverNode->value);
	} else if (receiverNode->type == LTRNUM) {
		r = dnk_build_number(receiverNode->value);
	} else if (receiverNode->type == NESTED) {
		r = dwlk_expr(receiverNode);
	} else {
		printf("Cannot send message to receiver of type: %d \n", receiverNode->type);
	}
	while((antiCrash++<100) && li->next) {
		li = li->next;
		msgnode = li->node;
		message = msgnode->value;
		argumentList = msgnode->nodes;
		args* a = A();
		args* aItem = a;
		if (argumentList) {
			tnode* node = argumentList->node;
			antiCrash=0;
			while((antiCrash++)<100) {
				obj* o = dwlk_expr(node);
				aItem->object = o;
				aItem->next = A();
				aItem = aItem->next;
				if (!argumentList->next) break;
				argumentList = argumentList->next;
				node = argumentList->node;
			}
		}
		result = dnk_send_message(r, message, a);
		r = result;
	}
	return result;
}	

obj* dwlk_assignment(tnode* node) {
	tlistitem* assignmentItems = node->nodes;
	tnode* assignee = assignmentItems->node;
	tlistitem* valueListItem = assignmentItems->next;
	tnode* value = valueListItem->node;
	obj* x = O();
	x = dwlk_expr(value);
	obj* result;
	if (assignee->modifier == 1) {
		result = dnk_assign_value_to_my(assignee->value, x);
	} else {
		result = dnk_assign_value(assignee->value, x);
	}
	return result;
}		

obj* dwlk_expr(tnode* node) {
	obj* result;
	if (node->type == LTRSTRING) {
		result = dnk_build_string(node->value);
	} else if (node->type == LTRBOOLTRUE) {
		result = dnk_build_bool(1);
	} else if (node->type == LTRBOOLFALSE) {
		result = dnk_build_bool(0);
	} else if (node->type == LTRNIL) {
		result = dnk_build_nil();
	} else if (node->type == LTRNUM) {
		result = dnk_build_number(node->value);
	} else if (node->type == CODEBLOCK) {
		result = dnk_build_block(node);
	} else if (node->type == REFERENCE) {
		if (node->modifier == 1) {
			result = dnk_find_in_my(node->value);
		} else {
			result = dnk_find(node->value);
		}
	} else if (node->type == EXPRMESSAGE) {
		result = dwlk_message(node);
	} else if (node->type == EXPRASSIGNMENT) {
		result = dwlk_assignment(node);
	} else if (node->type == RETURNFROMBLOCK) {
		result = dwlk_return(node);
	} else if (node->type == NESTED) {
		result = dwlk_expr(node->nodes->node);
	} else if (node->type == ENDOFPROGRAM) {
		printf("FIN.\n");
		exit(0);
	} else {
		printf("Runtime Error. Invalid parse node: %d %s \n", node->type,node->value);
		exit(1);
	}
	return result;
}

obj* dwlk_run(tnode* program) {
	obj* result = NULL;
	if (debug) tree(program, 0);
	int antiCrash = 0;
	tlistitem* li = program->nodes;
	while((antiCrash++)<100 && li) {
		tnode* node = li->node;
		if (!li->node) {
			printf("Missing parse node\n");
			exit(1);
		}
		result = dwlk_expr(node);
		if (!li->next) break;
		li = li->next;
	}
	return result;
}


