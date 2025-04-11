#include "citrine.h"

uint64_t ctr_gc_alloc;
uint64_t ctr_gc_memlimit;

/**
 * Heap Object, represents dynamic memory.
 */

struct memBlock {
	size_t size;
	void* space;
};

typedef struct memBlock memBlock;

memBlock*  memBlocks = NULL;
size_t     numberOfMemBlocks = 0;
size_t     maxNumberOfMemBlocks = 0;


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
void* ctr_heap_allocate( size_t size ) {
	void* slice_of_memory;
	size_t* block_width;
	int q = sizeof( size_t );
	size += q;
	/* Check whether we can afford to allocate this much */
	ctr_gc_alloc += size;
	if (ctr_gc_memlimit < ctr_gc_alloc) {
		printf( CTR_MERR_OOM, (unsigned long) size );
		exit(1);
	}
	/* Perform allocation and check result */
	slice_of_memory = calloc( 1, size );
	if ( slice_of_memory == NULL ) {
		printf( CTR_MERR_MALLOC, (unsigned long) size );
		exit(1);
	}
	/* Store the width of the memory block in the slice itself so we can always find it */
	block_width = (size_t*) slice_of_memory;
	*(block_width) = size;
	/* Now move the new memory pointer behind the blockwidth */
	slice_of_memory = (void*) ((char*) slice_of_memory + q);
	return slice_of_memory;
}

/**
 * Returns the current memory block number so you can rewind
 * the tracked memory allocator back to this point later on.
 * Used for deserializing.
 */
size_t ctr_heap_tracker_memoryblocknumber() {
	return numberOfMemBlocks;
}

/**
 * Rewinds memory block number to a previously stored
 * position, i.e. sequence number.
 */
size_t ctr_heap_tracker_rewind( size_t memoryBlockNumber ) {
	size_t i;
	i = 0;
	while ( numberOfMemBlocks > memoryBlockNumber) {
		ctr_heap_free(memBlocks[ --numberOfMemBlocks ].space);
		i ++;
	}
	return i;
}

/**
 * Allocates memory on heap and tracks it for clean-up when
 * the program ends.
 */
void* ctr_heap_allocate_tracked( size_t size ) {
	void* space;
	space = ctr_heap_allocate( size );
	if ( numberOfMemBlocks >= maxNumberOfMemBlocks ) {
		if ( memBlocks == NULL ) {
			memBlocks = ctr_heap_allocate( sizeof( memBlock ) );
			maxNumberOfMemBlocks = 1;
		} else {
			maxNumberOfMemBlocks += 10;
			memBlocks = ctr_heap_reallocate( memBlocks, ( sizeof( memBlock ) * ( maxNumberOfMemBlocks ) ) );
		}
	}
	memBlocks[ numberOfMemBlocks ].space = space;
	memBlocks[ numberOfMemBlocks ].size  = size;
	numberOfMemBlocks ++;
	return space;
}

/**
 * Reallocates tracked memory on heap.
 * You need to provide a tracking ID.
 */
void* ctr_heap_reallocate_tracked( size_t tracking_id, size_t size ) {
	void* space;
	space = memBlocks[ tracking_id ].space;
	space = ctr_heap_reallocate( space, size );
	memBlocks[ tracking_id ].space = space;
	memBlocks[ tracking_id ].size  = size;
	return space;
}

/**
 * Returns the latest tracking ID after tracking allocation.
 */
size_t ctr_heap_get_latest_tracking_id() {
	return numberOfMemBlocks - 1;
}

/**
 * Frees all tracked memory blocks.
 */
void ctr_heap_free_rest() {
	size_t i;
	for ( i = 0; i < numberOfMemBlocks; i ++) {
		ctr_heap_free( memBlocks[i].space );
	}
	ctr_heap_free( memBlocks );
	memBlocks = NULL;
	numberOfMemBlocks = 0;
	maxNumberOfMemBlocks = 0;
}


/**
 * Heap free memory
 *
 * Frees the memory pointed to by the specified pointer and deducts
 * the specified size from the allocation bookkeepting variable.
 *
 * @param void*     ptr  pointer to memory to be freed
 *
 * @return void
 */
int ctr_gc_clean_free = 0;
void ctr_heap_free( void* ptr ) {
	if (ptr == NULL) return;
	size_t* block_width;
	int q = sizeof( size_t );
	size_t size;
	/* find the correct size of this memory block and move pointer back */
	ptr = (void*) ((char*) ptr - q);
	if (ptr == NULL) return;
	block_width = (size_t*) ptr;
	size = *(block_width);
	if (ctr_gc_clean_free) {
		memset(ptr,0,size);
	}
	if (ptr != NULL) free(ptr);
	if (size > ctr_gc_alloc) {
		ctr_print_error("[WARNING] Freeing more memory than allocated.", -1);
		ctr_gc_alloc = 0;
	} else {
		ctr_gc_alloc -= size;
	}
}

/**
 * Memory Management Adjust Memory Block Size (re-allocation)
 * Re-allocates Memory Block.
 *
 * Given the old pointer, the desired size, the original size and
 * the purpose for allocation, this function will attempt to
 * re-allocate the memory block.
 */
void* ctr_heap_reallocate(void* oldptr, size_t size ) {

	char* nptr;
	size_t  old_size;
	size_t* block_width;

	/* correct the required size new block to include block width */
	int q = sizeof( size_t );
	size += q;
	/* move the pointer back to begin of block */
	oldptr = (void*) ((char*) oldptr - q);
	/* read memory size at beginning of old block */
	block_width = (size_t*) oldptr;
	old_size = *(block_width);

	/* if somehow the requested size is less than the old size */
	/* otherwise upon copying memory contents we will cross    */
	/* boundaries.                                             */
	if (size <= old_size) {
		return (void*) ((char*) oldptr + q);
	}

	/* update the ledger */
	ctr_gc_alloc = ( ctr_gc_alloc - old_size ) + size;

	/* re-allocate memory */
	nptr = calloc( 1, size );
	if ( nptr == NULL ) {
		printf( CTR_MERR_MALLOC, (unsigned long) size );
		exit(1);
	}
	memcpy( nptr, oldptr, old_size );
	if (oldptr != NULL) free(oldptr);
	
	/* store the size of the new block at the beginning */
	block_width = (size_t*) nptr;
	*(block_width) = size;
	/* 'hop' the memory pointer over the block width part */
	nptr = (void*) ((char*) nptr + q);

	return (void*) nptr;
}

/**
 * @internal
 *
 * Casts a string object to a cstring.
 * Given an object with a string value, this function
 * will return a C-string representing the bytes contained
 * in the String Object. This function will explicitly end
 * the returned set of bytes with a \0 byte for use
 * in traditional C string functions.
 *
 * Warning: this function 'leaks' memory, you have to ctr_heap_free() it.
 * It will allocate the necessary resources to store the string.
 * To free this memory you'll need to call ctr_heap_free
 * passing the pointer and the number of bytes ( value.svalue->vlen ).
 *
 * @note
 * This function removes NULL-bytes from the resulting C-string
 * to avoid NUL-byte-injections.
 *
 * @param ctr_object* stringObject CtrString object instance to cast
 *
 * @return char*
 */
char* ctr_heap_allocate_cstring( ctr_object* stringObject ) {
	char*    cstring;
	char*    stringBytes;
	ctr_size length;
	ctr_size i, j;
	stringBytes = stringObject->value.svalue->value;
	length      = stringObject->value.svalue->vlen;
	cstring     = ctr_heap_allocate( ( length + 1 ) * sizeof( char ) );
	j = 0;
	for (i = 0; i < length; i++) {
		if ( stringBytes[i] == '\0' ) {
			continue;
		}
		cstring[j++] = stringBytes[i];
	}
	cstring[j] = '\0';
	return cstring;
}