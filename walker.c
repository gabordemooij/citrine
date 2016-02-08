#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>

#include "citrine.h"

/**
 * CTRWalkerReturn
 *
 * Returns from a block of code.
 */
ctr_object* ctr_cwlk_return(ctr_tnode* node) {
	char wasReturn = 0;
	ctr_tlistitem* li;
	ctr_object* e;
	if (!node->nodes) {
		printf("Invalid return expression.\n");
		exit(1);
	}
	li = node->nodes;
	if (!li->node) {
		printf("Invalid return expression 2.\n");
		exit(1);
	} 
	e = ctr_cwlk_expr(li->node, &wasReturn);
	return e;
}

/**
 * CTRWalkerMessage
 *
 * Processes a message sending operation.
 */
ctr_object* ctr_cwlk_message(ctr_tnode* paramNode) {
	char wasReturn = 0;
	ctr_object* result;
	ctr_tlistitem* eitem = paramNode->nodes;
	ctr_tnode* receiverNode = eitem->node;
	ctr_tnode* msgnode;
	ctr_tlistitem* li = eitem;
	char* message;
	ctr_tlistitem* argumentList;
	ctr_object* r;
	switch (receiverNode->type) {
		case CTR_AST_NODE_REFERENCE:
			if (receiverNode->modifier == 1) {
				r = ctr_find_in_my(ctr_build_string(receiverNode->value, receiverNode->vlen));
			} else {
				r = ctr_find(ctr_build_string(receiverNode->value, receiverNode->vlen));
			}
			if (!r) {
				exit(1);
			}
			break;
		case CTR_AST_NODE_LTRNIL:
			r = ctr_build_nil();
			break;
		case CTR_AST_NODE_LTRBOOLTRUE:
			r = ctr_build_bool(1);
			break;
		case CTR_AST_NODE_LTRBOOLFALSE:
			r = ctr_build_bool(0);
			break;
		case CTR_AST_NODE_LTRSTRING:
			r = ctr_build_string(receiverNode->value, receiverNode->vlen);
			break;
		case CTR_AST_NODE_LTRNUM:
			r = ctr_build_number_from_string(receiverNode->value, receiverNode->vlen);
			break;
		case CTR_AST_NODE_NESTED:
			r = ctr_cwlk_expr(receiverNode, &wasReturn);
			break;
		case CTR_AST_NODE_CODEBLOCK:
			r = ctr_build_block(receiverNode);
			break;
		default:
			printf("Cannot send message to receiver of type: %d \n", receiverNode->type);
			break;
	}
	while(li->next) {
		ctr_argument* a;
		ctr_argument* aItem;
		ctr_size l;
		li = li->next;
		msgnode = li->node;
		message = msgnode->value;
		l = msgnode->vlen;
		argumentList = msgnode->nodes;
		a = CTR_CREATE_ARGUMENT();
		aItem = a;
		if (argumentList) {
			ctr_tnode* node;
			node = argumentList->node;
			while(1) {
				ctr_object* o = ctr_cwlk_expr(node, &wasReturn);
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

/**
 * CTRWalkerAssignment
 *
 * Processes an assignment operation.
 */
ctr_object* ctr_cwlk_assignment(ctr_tnode* node) {
	char wasReturn = 0;
	ctr_tlistitem* assignmentItems = node->nodes;
	ctr_tnode* assignee = assignmentItems->node;
	ctr_tlistitem* valueListItem = assignmentItems->next;
	ctr_tnode* value = valueListItem->node;
	ctr_object* x = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	ctr_object* result;
	x = ctr_cwlk_expr(value, &wasReturn);
	if (assignee->modifier == 1) {
		result = ctr_assign_value_to_my(ctr_build_string(assignee->value, assignee->vlen), x);
	} else if (assignee->modifier == 2) {
		result = ctr_assign_value_to_local(ctr_build_string(assignee->value, assignee->vlen), x);
	} else {
		result = ctr_assign_value(ctr_build_string(assignee->value, assignee->vlen), x);
	}
	return result;
}	

/**
 * CTRWalkerExpression
 *
 * Processes an expression.
 */
ctr_object* ctr_cwlk_expr(ctr_tnode* node, char* wasReturn) {
	ctr_object* result;
	switch (node->type) {
		case CTR_AST_NODE_LTRSTRING:
			result = ctr_build_string(node->value, node->vlen);
			break;
		case CTR_AST_NODE_LTRBOOLTRUE:
			result = ctr_build_bool(1);
			break;
		case CTR_AST_NODE_LTRBOOLFALSE:
			result = ctr_build_bool(0);
			break;
		case CTR_AST_NODE_LTRNIL:
			result = ctr_build_nil();
			break;
		case CTR_AST_NODE_LTRNUM:
			result = ctr_build_number_from_string(node->value, node->vlen);
			break;
		case CTR_AST_NODE_CODEBLOCK:
			result = ctr_build_block(node);
			break;
		case CTR_AST_NODE_REFERENCE:
			if (node->modifier == 1) {
				result = ctr_find_in_my(ctr_build_string(node->value, node->vlen));
			} else {
				result = ctr_find(ctr_build_string(node->value, node->vlen));
			}
			break;
		case CTR_AST_NODE_EXPRMESSAGE:
			result = ctr_cwlk_message(node);
			break;
		case CTR_AST_NODE_EXPRASSIGNMENT:
			result = ctr_cwlk_assignment(node);
			break;
		case CTR_AST_NODE_RETURNFROMBLOCK:
			result = ctr_cwlk_return(node);
			*wasReturn = 1;
			break;
		case CTR_AST_NODE_NESTED:
			result = ctr_cwlk_expr(node->nodes->node, wasReturn);
			break;
		case CTR_AST_NODE_ENDOFPROGRAM:
			if (CtrStdError) {
				printf("Uncatched error has occurred.\n");
				if (CtrStdError->info.type == CTR_OBJECT_TYPE_OTSTRING) {
					fwrite(CtrStdError->value.svalue->value, sizeof(char), CtrStdError->value.svalue->vlen, stdout);
					printf("\n");
				}
				exit(1);
			}
			result = ctr_build_nil();
			break;
		default:
			printf("Runtime Error. Invalid parse node: %d %s \n", node->type,node->value);
			exit(1);
			break;
	}
	return result;
}

/**
 * CTRWalkerRunBlock
 *
 * Processes the execution of a block of code.
 */
ctr_object* ctr_cwlk_run(ctr_tnode* program) {
	ctr_object* result = NULL;
	char wasReturn = 0;
	ctr_tlistitem* li;
	if (ctr_mode_debug) ctr_internal_debug_tree(program, 0);
	li = program->nodes;
	while(li) {
		ctr_tnode* node = li->node;
		if (!li->node) {
			printf("Missing parse node\n");
			exit(1);
		}
		wasReturn = 0;
		result = ctr_cwlk_expr(node, &wasReturn);
		if (!li->next) break;
		li = li->next;
	}
	if (wasReturn == 0) result = NULL;
	return result;
}


