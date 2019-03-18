#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>

#include "citrine.h"

ctr_tnode* ctr_callstack[301];
uint8_t ctr_callstack_index;


/**
 * CTRWalkerReturn
 *
 * Returns from a block of code.
 */
ctr_object* ctr_cwlk_return(ctr_tnode* node) {
	char wasReturn = 0;
	ctr_tlistitem* li;
	ctr_object* e;
	if (ctr_flag_sandbox && ++ctr_sandbox_steps>CTR_MAX_STEPS_LIMIT) exit(1);
	if (!node->nodes) {
		fprintf(stderr, CTR_ERR_RET );
		exit(1);
	}
	li = node->nodes;
	if (!li->node) {
		fprintf(stderr, CTR_ERR_RET );
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
	int sticky = 0;
	char wasReturn = 0;
	int literal = 1;
	ctr_object* keys[40];
	int key_index = 0;
	ctr_object* result;
	ctr_tlistitem* eitem = paramNode->nodes;
	ctr_tnode* receiverNode = eitem->node;
	ctr_tnode* msgnode;
	ctr_tlistitem* li = eitem;
	char* message;
	ctr_tlistitem* argumentList;
	ctr_object* r;
	ctr_object* recipientName = NULL;
	if (ctr_flag_sandbox && ++ctr_sandbox_steps>CTR_MAX_STEPS_LIMIT) exit(1);
	switch (receiverNode->type) {
		case CTR_AST_NODE_REFERENCE:
			literal = 0;
			recipientName = ctr_build_string(receiverNode->value, receiverNode->vlen);
			recipientName->info.sticky = 1;
			if (CtrStdFlow == NULL) {
				ctr_callstack[ctr_callstack_index++] = receiverNode;
			}
			if (receiverNode->modifier == 1) {
				r = ctr_find_in_my(recipientName);
			} else {
				r = ctr_find(recipientName);
			}
			if (CtrStdFlow == NULL) {
				ctr_callstack_index--;
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
			fprintf(stderr, CTR_ERR_SEND, receiverNode->type);
			break;
	}
	int ctr_assume_message_level = ctr_in_message;
	int ctr_is_chain = 0;
	while(li->next) {
		if (ctr_flag_sandbox && ++ctr_sandbox_steps>CTR_MAX_STEPS_LIMIT) exit(1);
		if (ctr_is_chain) {
			ctr_in_message++;
		}
		ctr_is_chain++;
		ctr_argument* a;
		ctr_argument* aItem;
		ctr_size l;
		li = li->next;
		msgnode = li->node;
		message = msgnode->value;
		l = msgnode->vlen;
		if (CtrStdFlow == NULL) {
			ctr_callstack[ctr_callstack_index++] = msgnode;
		}
		argumentList = msgnode->nodes;
		a = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		aItem = a;
		aItem->object = CtrStdNil;
		sticky = r->info.sticky;
		r->info.sticky = 1;
		if (literal) {
			keys[key_index++] = ctr_gc_internal_pin(r);
		}
		if (argumentList) {
			ctr_tnode* node;
			node = argumentList->node;
			while(1) {
				ctr_in_message++;
				ctr_object* o = ctr_cwlk_expr(node, &wasReturn);
				ctr_in_message--;
				aItem->object = o;
				keys[key_index++] = ctr_gc_internal_pin(o);
				
				if (key_index > 39) {
					printf( CTR_ERR_KEYINX );
					exit(1);
				}
				/* we always send at least one argument, note that if you want to modify the argumentList, be sure to take this into account */
				/* there is always an extra empty argument at the end */
				aItem->next = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
				aItem = aItem->next;
				aItem->object = CtrStdNil;
				if (!argumentList->next) break;
				argumentList = argumentList->next;
				node = argumentList->node;
			}
		}
		result = ctr_send_message(r, message, l, a);
		r->info.sticky = sticky;
		aItem = a;
		if (CtrStdFlow == NULL) {
			ctr_callstack_index --;
		}
		while(aItem->next) {
			a = aItem;
			aItem = aItem->next;
			ctr_internal_object_delete_property(ctr_contexts[ctr_context_id], keys[--key_index], CTR_CATEGORY_PRIVATE_PROPERTY);
			ctr_heap_free( a );
		}
		ctr_heap_free( aItem );
		if (literal) {
			ctr_internal_object_delete_property( ctr_contexts[ctr_context_id], keys[--key_index], CTR_CATEGORY_PRIVATE_PROPERTY );
		}
		r = result;
	}
	ctr_in_message -= (ctr_is_chain - 1);
	if (ctr_in_message != ctr_assume_message_level) {
		printf( CTR_ERR_ANOMALY );
		exit(1);
	}
	if (recipientName) recipientName->info.sticky = 0;
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
	ctr_object* x;
	ctr_object* result;
	if (ctr_flag_sandbox && ++ctr_sandbox_steps>CTR_MAX_STEPS_LIMIT) exit(1);
	if (CtrStdFlow == NULL) {
		ctr_callstack[ctr_callstack_index++] = assignee;
	}
	x = ctr_cwlk_expr(value, &wasReturn);
	if (assignee->modifier == 1) {
		result = ctr_assign_value_to_my(ctr_build_string(assignee->value, assignee->vlen), x);
	} else if (assignee->modifier == 2) {
		result = ctr_assign_value_to_local(ctr_build_string(assignee->value, assignee->vlen), x);
	} else {
		result = ctr_assign_value(ctr_build_string(assignee->value, assignee->vlen), x);
	}
	if (CtrStdFlow == NULL) {
		ctr_callstack_index--;
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
	uint8_t i;
	int line;
	char* currentProgram = "?";
	ctr_tnode* stackNode;
	ctr_source_map* mapItem;
	if (ctr_flag_sandbox && ++ctr_sandbox_steps>CTR_MAX_STEPS_LIMIT) exit(1);
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
			if (CtrStdFlow == NULL) {
				ctr_callstack[ctr_callstack_index++] = node;
			}
			if (node->modifier == 1) {
				result = ctr_find_in_my(ctr_build_string(node->value, node->vlen));
			} else {
				result = ctr_find(ctr_build_string(node->value, node->vlen));
			}
			if (CtrStdFlow == NULL) {
				ctr_callstack_index--;
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
			if (CtrStdFlow && CtrStdFlow != CtrStdExit && ctr_cwlk_subprogram == 0) {
				fprintf(stderr, CTR_ERR_UNCAUGHT );
				if (CtrStdFlow->info.type == CTR_OBJECT_TYPE_OTSTRING) {
					fwrite(CtrStdFlow->value.svalue->value, sizeof(char), CtrStdFlow->value.svalue->vlen, stderr);
					fprintf(stderr,"\n");
				}
				for ( i = ctr_callstack_index; i > 0; i--) {
					fprintf(stderr,"#%d ", i);
					stackNode = ctr_callstack[i-1];
					fwrite(stackNode->value, sizeof(char), stackNode->vlen, stderr);
					mapItem = ctr_source_map_head;
					line = -1;
					while(mapItem) {
						if (line == -1 && mapItem->node == stackNode) {
							line = mapItem->line;
						}
						if (line > -1 && mapItem->node->type == CTR_AST_NODE_PROGRAM) {
							currentProgram = mapItem->node->value;
							fprintf(stderr," (%s: %d)", currentProgram, line+1);
							break;
						}
						mapItem = mapItem->next;
					}
					fprintf(stderr,"\n");
				}
			}
			result = ctr_build_nil();
			break;
		default:
			fprintf(stderr, CTR_ERR_NODE, node->type,node->value);
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
	li = program->nodes;
	while(li) {
		ctr_tnode* node = li->node;
		if (!li->node) {
			fprintf(stderr, CTR_ERR_MISSING );
			exit(1);
		}
		wasReturn = 0;
		result = ctr_cwlk_expr(node, &wasReturn);
		if ( wasReturn ) {
			break;
		}
		/* Perform garbage collection cycle */
		if ( ( ( ctr_gc_mode & 1 ) && ctr_gc_alloc > ( ctr_gc_memlimit * 0.8 ) ) || ctr_gc_mode & 4 ) {
			ctr_gc_internal_collect();
		}
		if (!li->next) break;
		li = li->next;
	}
	if (wasReturn == 0) result = NULL;
	return result;
}


