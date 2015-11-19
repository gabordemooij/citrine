#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>

#include "citrine.h"

ctr_object* error;

ctr_object* cwlk_return(tnode* node) {
	
	if (!node->nodes) {
		printf("Invalid return expression.\n");
		exit(1);
	}
	tlistitem* li = node->nodes;
	if (!li->node) {
		printf("Invalid return expression 2.\n");
		exit(1);
	} 
	ctr_object* e = ctr_internal_create_object(OTOBJECT);
	e =  cwlk_expr(li->node);
	return e;
}

ctr_object* cwlk_message(tnode* paramNode) {
	ctr_object* result;
	tlistitem* eitem = paramNode->nodes;
	tnode* receiverNode = eitem->node;
	tnode* msgnode;
	tlistitem* li = eitem;
	char* message;
	tlistitem* argumentList;
	ctr_object* r;
	if (receiverNode->type == REFERENCE) {
		r = ctr_find(ctr_build_string(receiverNode->value, receiverNode->vlen));
		if (!r) {
			exit(1);
		}
	} else if (receiverNode->type == LTRNIL ) {
		r = ctr_build_nil();
	} else if (receiverNode->type == LTRBOOLTRUE ) {
		r = ctr_build_bool(1);
	} else if (receiverNode->type == LTRBOOLFALSE ) {
		r = ctr_build_bool(0);
	} else if (receiverNode->type == LTRSTRING ) {
		r = ctr_build_string(receiverNode->value, receiverNode->vlen);
	} else if (receiverNode->type == LTRNUM) {
		r = ctr_build_number(receiverNode->value);
	} else if (receiverNode->type == NESTED) {
		r = cwlk_expr(receiverNode);
	} else if (receiverNode->type == CODEBLOCK) {
		r = ctr_build_block(receiverNode);
	} else {
		printf("Cannot send message to receiver of type: %d \n", receiverNode->type);
	}
	while(li->next) {
		li = li->next;
		msgnode = li->node;
		message = msgnode->value;
		long l = msgnode->vlen;
		argumentList = msgnode->nodes;
		args* a = CTR_CREATE_ARGUMENT();
		args* aItem = a;
		if (argumentList) {
			tnode* node = argumentList->node;
			while(1) {
				ctr_object* o = cwlk_expr(node);
				aItem->object = o;
				aItem->next = CTR_CREATE_ARGUMENT();
				aItem = aItem->next;
				if (!argumentList->next) break;
				argumentList = argumentList->next;
				node = argumentList->node;
			}
		}
		CTR_DEBUG_STR("Sending message: %s \n",message,l);
		result = ctr_send_message(r, message, l, a);
		r = result;
	}
	return result;
}	

ctr_object* cwlk_assignment(tnode* node) {
	tlistitem* assignmentItems = node->nodes;
	tnode* assignee = assignmentItems->node;
	tlistitem* valueListItem = assignmentItems->next;
	tnode* value = valueListItem->node;
	ctr_object* x = ctr_internal_create_object(OTOBJECT);
	x = cwlk_expr(value);
	ctr_object* result;
	if (assignee->modifier == 1) {
		result = ctr_assign_value_to_my(ctr_build_string(assignee->value, assignee->vlen), x);
	} else {
		result = ctr_assign_value(ctr_build_string(assignee->value, assignee->vlen), x);
	}
	return result;
}		

ctr_object* cwlk_expr(tnode* node) {
	ctr_object* result;
	if (node->type == LTRSTRING) {
		result = ctr_build_string(node->value, node->vlen);
	} else if (node->type == LTRBOOLTRUE) {
		result = ctr_build_bool(1);
	} else if (node->type == LTRBOOLFALSE) {
		result = ctr_build_bool(0);
	} else if (node->type == LTRNIL) {
		result = ctr_build_nil();
	} else if (node->type == LTRNUM) {
		result = ctr_build_number(node->value);
	} else if (node->type == CODEBLOCK) {
		result = ctr_build_block(node);
	} else if (node->type == REFERENCE) {
		if (node->modifier == 1) {
			result = ctr_find_in_my(ctr_build_string(node->value, node->vlen));
		} else {
			result = ctr_find(ctr_build_string(node->value, node->vlen));
		}
	} else if (node->type == EXPRMESSAGE) {
		result = cwlk_message(node);
	} else if (node->type == EXPRASSIGNMENT) {
		result = cwlk_assignment(node);
	} else if (node->type == RETURNFROMBLOCK) {
		result = cwlk_return(node);
	} else if (node->type == NESTED) {
		result = cwlk_expr(node->nodes->node);
	} else if (node->type == ENDOFPROGRAM) {
		if (error) {
			printf("Uncatched error has occurred.\n");
			if (error->info.type == OTSTRING) {
				fwrite(error->value.svalue->value, sizeof(char), error->value.svalue->vlen, stdout);
				printf("\n");
			}
			exit(1);
		}
		result = ctr_build_nil();
	} else {
		printf("Runtime Error. Invalid parse node: %d %s \n", node->type,node->value);
		exit(1);
	}
	return result;
}

ctr_object* cwlk_run(tnode* program) {
	ctr_object* result = NULL;
	if (debug) tree(program, 0);
	tlistitem* li = program->nodes;
	while(li) {
		tnode* node = li->node;
		if (!li->node) {
			printf("Missing parse node\n");
			exit(1);
		}
		result = cwlk_expr(node);
		if (!li->next) break;
		li = li->next;
	}
	return result;
}


