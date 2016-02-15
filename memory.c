#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include "citrine.h"

/**
 * Memory Management Allocate
 * Allocates a block of memory of a certain size
 * for a specified purpose.
 *
 * List of purposes (what parameter):
 *
 * 0: binary block, just allocate
 * 1: allocate memory for tree node for AST serialization, adds to addressbook
 * 2: allocate memory for tree list for AST serialization, adds to addressbook
 * 3: allocate memory for program entry point (PEP) for AST, adds to addressbook
 */
char* ctr_malloc(uintptr_t size, int what) {
	char* beginBlock;
	char* xptr;
	if (ctr_malloc_mode == 0) {
		ctr_malloc_measured_size_addressbook += (sizeof(uintptr_t) * 2);
		ctr_malloc_measured_size_code += size;
		return (char*) calloc(size,sizeof(char));
	}
	if (!ctr_malloc_chunk) {
		ctr_default_header = malloc(sizeof(ctr_ast_header));
		strncpy(ctr_default_header->version,"CITR000001",10);
		ctr_default_header->num_of_swizzles = 0;
		ctr_malloc_chunk = (char*) malloc((ctr_malloc_measured_size_code+ctr_malloc_measured_size_addressbook)*sizeof(char));
		if (!ctr_malloc_chunk) exit(1);
		ctr_malloc_chunk_pointer = ctr_malloc_measured_size_addressbook;
		ctr_default_header->size_of_address_book = ctr_malloc_measured_size_addressbook;/*<----*/
		ctr_malloc_swizzle_adressbook = ((uintptr_t*) (ctr_malloc_chunk + sizeof(ctr_ast_header)));
		ctr_default_header->start_block = (uintptr_t) ctr_malloc_chunk;
		ctr_default_header->program_entry_point = 0;
	}
	beginBlock = ctr_malloc_chunk + ctr_malloc_chunk_pointer;
	ctr_malloc_chunk_pointer += size;
	xptr = beginBlock;
	if (what != 1 && what != 2 && what != 3) {
		return xptr;
	}
	if (what == 1 || what == 3) {
		ctr_tnode tmp0;
		ctr_tnode tmp;
		*(ctr_malloc_swizzle_adressbook) = (uintptr_t) xptr + (uintptr_t) ((uintptr_t) &(tmp0.value) - (uintptr_t) &tmp0);
		ctr_default_header->num_of_swizzles++;
		ctr_malloc_swizzle_adressbook += 1;
		*(ctr_malloc_swizzle_adressbook) = (uintptr_t) xptr + (uintptr_t) ((uintptr_t) &(tmp.nodes) - (uintptr_t) &tmp);
		ctr_default_header->num_of_swizzles++;
		ctr_malloc_swizzle_adressbook += 1;
	}
	if (what == 2) {
		ctr_tlistitem tmp2;
		ctr_tlistitem tmp3;
		*(ctr_malloc_swizzle_adressbook) = (uintptr_t) xptr + (uintptr_t) ((uintptr_t) &(tmp2.node) - (uintptr_t) &tmp2);
		ctr_malloc_swizzle_adressbook += 1;
		ctr_default_header->num_of_swizzles++;
		*(ctr_malloc_swizzle_adressbook) = (uintptr_t) 	xptr + (uintptr_t) ((uintptr_t) &(tmp3.next) - (uintptr_t) &tmp3);
		ctr_malloc_swizzle_adressbook += 1;
		ctr_default_header->num_of_swizzles++;
	}
	if (what == 3) {
		ctr_default_header->program_entry_point = (uintptr_t) xptr;
	}

	return xptr;
}

/**
 * Memory Management Adjust Memory Block Size (re-allocation)
 * Re-allocates Memory Block.
 *
 * Given the old pointer, the desired size, the original size and
 * the purpose for allocation, this function will attempt to
 * re-allocate the memory block.
 */
void* ctr_realloc(void* oldptr, uintptr_t size, uintptr_t old_size, int what) {
	char* nptr;
	nptr = ctr_malloc(size, what);
	memcpy(nptr, oldptr, old_size);
	return (void*) nptr;
}
