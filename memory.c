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

char* ctr_pool_alloc( ctr_size podSize );
void ctr_pool_dealloc( void* ptr );
void ctr_pool_init();
int ctr_pool_bucket( ctr_size size );

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
	size = ctr_pool_bucket( size );
	/* Check whether we can afford to allocate this much */
	ctr_gc_alloc += size;
	if (ctr_gc_memlimit < ctr_gc_alloc) {
		printf( CTR_MERR_OOM, (unsigned long) size );
		exit(1);
	}
	/* Perform allocation and check result */
	slice_of_memory = ctr_pool_alloc( size );
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
void ctr_heap_free( void* ptr ) {
	size_t* block_width;
	int q = sizeof( size_t );
	size_t size;
	/* find the correct size of this memory block and move pointer back */
	ptr = (void*) ((char*) ptr - q);
	block_width = (size_t*) ptr;
	size = *(block_width);
	ctr_pool_dealloc( ptr );
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
void* ctr_heap_reallocate(void* oldptr, size_t size ) {

	char* nptr;
	size_t  old_size;
	size_t* block_width;

	/* correct the required size new block to include block width */
	int q = sizeof( size_t );
	size += q;
	size = ctr_pool_bucket( size );
	/* move the pointer back to begin of block */
	oldptr = (void*) ((char*) oldptr - q);
	/* read memory size at beginning of old block */
	block_width = (size_t*) oldptr;
	old_size = *(block_width);

	/* if somehow the requested size is less than the old size */
	/* (because of bucketing) just return same memory block    */
	/* otherwise upon copying memory contents we will cross    */
	/* boundaries.                                             */
	if (size <= old_size) {
		return (void*) ((char*) oldptr + q);
	}

	/* update the ledger */
	ctr_gc_alloc = ( ctr_gc_alloc - old_size ) + size;

	/* re-allocate memory */
	nptr = ctr_pool_alloc( size );
	memcpy( nptr, oldptr, old_size );
	ctr_pool_dealloc(oldptr);

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

/**
 * Pool Allocator
 *
 * The Pool Allocator improves the performance of memory allocation in Citrine.
 * To activate the pool allocator, set the 4th bit (i.e. 8) of the Broom Garbage Collector
 * using 'Broom mode:'.
 */
int       usePools = 0;
char*     spodmem;
char*     mpodmem;
char*     lpodmem;
char**    freeslist;
char**    freemlist;
char**    freellist;
ctr_size  freespods = 0;
ctr_size  freempods = 0;
ctr_size  freelpods = 0;
ctr_size  spods = 0;
ctr_size  mpods = 0 ;
ctr_size  lpods = 0 ;
ctr_size  spodCount = 0;
ctr_size  mpodCount = 0;
ctr_size  lpodCount = 0;
ctr_size  spod = 32;
ctr_size  mpod = 64;
ctr_size  lpod = 128;


/**
 * Initializes the Pool Allocator with the specified memory limit.
 * By default the Pool Allocator will use 50% of the specified memory
 * for pool allocation.
 */
void ctr_pool_init( ctr_size pool ) {
	if (usePools) return; /* You cannot init twice */
	usePools = 1;
	ctr_size poolSize = pool / 3;
	spods = (poolSize / spod) - 1;
	mpods = (poolSize / mpod) - 1;
	lpods = (poolSize / lpod) - 1;
	spodmem =  malloc( poolSize );
	mpodmem =  malloc( poolSize );
	lpodmem =  malloc( poolSize );
	if (spodmem == NULL || mpodmem == NULL || lpodmem == NULL) {
		printf( CTR_MERR_POOL );
		exit(1);
	}
	freeslist = (char**) malloc(sizeof(char**) * spods);
	freemlist = (char**) malloc(sizeof(char**) * mpods);
	freellist = (char**) malloc(sizeof(char**) * lpods);
}

/**
 * Given the size of the requested memory this function will return the
 * resulting POD size to be used. There are 3 pod sizes available.
 *
 * <= 32 bytes:                  Small sized POD (spod)
 * >  32 bytes and <= 64  bytes  Medium sized POD (mpod)
 * >  64 bytes and <= 128 bytes  Large sized POD (lpod)
 *
 * If the size fits in one of these pods (<= lpod) but no pods are available
 * anymore in the pool, the size will be adjusted to size + 1, so
 * 32 will become 33. That way, the pool will be able to separate
 * these custom allocations from pods. Otherwise you will get double frees.
 */
int ctr_pool_bucket( ctr_size size ) {
	if ( size > 0 && size <= spod && spodCount<spods) {
		size = spod;
	} else if ( size > spod && size <= mpod && mpodCount<mpods) {
		size = mpod;
	} else if ( size > mpod && size <= lpod && lpodCount<lpods) {
		size = lpod;
	} else if (size == spod || size == mpod || size == lpod) {
		size = size + 1; /* identify as custom size (avoid ambiguity 32 -> 33) otherwise double free(). */
	}
	return size;
}

/**
 * Returns a pointer to a block of memory allocated using either
 * the pool or the OS.
 */
char* ctr_pool_alloc( ctr_size podSize ) {
	if (!usePools) return (char*) calloc(podSize, 1);
	char* memblock = NULL;
	
	if (podSize == spod && freespods>0) {
		memblock = (char*) (freeslist[--freespods]);
	} else if (podSize == spod && spodCount<spods) {
		memblock = (spodmem + ((spodCount++)*spod));
	} else if (podSize == mpod && freempods>0) {
		memblock = (char*) (freemlist[--freempods]);
	} else if (podSize == mpod && mpodCount<mpods) {
		memblock = (mpodmem + ((mpodCount++)*mpod));
	} else if (podSize == lpod && freelpods>0) {
		memblock = (char*) (freellist[--freelpods]);
	} else if (podSize == lpod && lpodCount<lpods) {
		memblock = (lpodmem + ((lpodCount++)*lpod));
	} else {
		memblock = (char*) calloc(podSize, 1);
	}
	return (char*) memblock;
}

/**
 * Deallocates memory using the pool or the OS.
 */
void ctr_pool_dealloc( void* ptr ) {
	if (!usePools) {
		free(ptr);
		return;
	}
	ctr_size podSize;
	podSize = *((ctr_size*) ptr);
	if (podSize == spod && freespods<spods) {
		freeslist[freespods++] = (char*) ptr;
		memset(ptr,0,spod);
	} else if (podSize == mpod && freempods<mpods) {
		freemlist[freempods++] = (char*) ptr;
		memset(ptr,0,mpod);
	} else if (podSize == lpod && freelpods<lpods) {
		freellist[freelpods++] = (char*) ptr;
		memset(ptr,0,lpod);
	} else {
		free(ptr);
	}
}
