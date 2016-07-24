#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include "citrine.h"

/**
 * Heap Object, represents dynamic memory.
 */

/**
 * Heap allocate raw memory
 * Allocates a slice of memory having the specified size in bytes.
 * The memory will be zeroed (calloc is used).
 *
 * If the specified number of bytes cannot be allocated, the program
 * will end with exit 1.
 *
 * If the specified number of bytes causes the total number of allocated
 * bytes to exceed the GC thresold, the garbage collector will attempt to free
 * memory.
 *
 * If the specified number of bytes causes the total number of allocated
 * bytes to exceed the predetermined memory limit, the program will exit with
 * return code 1.
 *
 * This function will track the allocated bytes to monitor memory
 * management.
 *
 * @param uintptr_t size memory size
 *
 * @return void*
 */
void* ctr_heap_allocate( uintptr_t size ) {

	void* sliceOfMemory;

	/* Check whether we can afford to allocate this much */
	ctr_gc_alloc += size;
	if (ctr_gc_memlimit < ctr_gc_alloc) {
		printf( "Out of memory. Failed to allocate %lu bytes.\n", size );
		exit(1);
	}

	/* Perform allocation and check result */
	sliceOfMemory = calloc( size, 1 );

	if ( sliceOfMemory == NULL ) {
		printf( "Out of memory. Failed to allocate %lu bytes (malloc failed). \n", size );
	}

	/* Perform garbage collection cycle */
	if ( ( ctr_gc_mode & 1 ) && ctr_gc_alloc > ( ctr_gc_memlimit * 0.8 ) ) {
		ctr_gc_internal_collect();
	}

	return sliceOfMemory;
}

/**
 * Heap free memory
 *
 * Frees the memory pointed to by the specified pointer and deducts
 * the specified size from the allocation bookkeepting variable.
 *
 * @param void*     ptr  pointer to memory to be freed
 * @param uintptr_t size number of bytes to deduct from usage counter
 *
 * @return void
 */
void ctr_heap_free( void* ptr, uintptr_t size ) {
	free( ptr );
	ctr_gc_alloc -= size;
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
	ctr_gc_alloc -= old_size;
	nptr = ctr_heap_allocate( size );
	memcpy(nptr, oldptr, old_size);
	return (void*) nptr;
}
