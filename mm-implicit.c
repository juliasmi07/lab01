/*
 * mm-implicit.c - The best malloc package EVAR!
 *
 * TODO (bug): mm_realloc and mm_calloc don't seem to be working...
 * TODO (bug): The allocator doesn't re-use space very well...
 *
 * This is the only file you edit and submit. Every function below has a
 * comment describing what it does. Work through the checkpoints in the
 * writeup in order:
 *
 *   Checkpoint 1: implement mm_realloc and mm_calloc
 *   Checkpoint 2: split blocks in mm_malloc
 *   Checkpoint 3: implement coalesce_free_blocks
 *
 * A note on C: this file is C, not C++. Two things in it are legal here
 * but not in C++:
 *   - Adding an integer to `void *` moves it that many BYTES. So
 *     `(void *) block + 32` is the address 32 bytes after `block`, no
 *     matter what type `block` points to. (Adding to a `block_t *` would
 *     instead move by multiples of sizeof(block_t).)
 *   - A `void *` converts to any other pointer type without a cast, so
 *     `block_t *b = mem_sbrk(32);` compiles.
 */

#include <stdint.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/** The required alignment of heap payloads */
const size_t ALIGNMENT = 2 * sizeof(size_t);

/**
 * The layout of each block allocated on the heap.
 *
 *   block -> +----------------+
 *            | header (8 B)   |  size of the WHOLE block (header included)
 *            +----------------+  | allocated bit (1 = allocated, 0 = free)
 *            | payload ...    |  <- the pointer mm_malloc returns
 *            |                |
 *            +----------------+  <- the next block starts here:
 *                                   (void *) block + get_size(block)
 *
 * sizeof(block_t) is 8: the payload array has no fixed length, so it
 * takes no space in the struct. It only gives a name to the address right
 * after the header.
 */
typedef struct {
    /** The size of the block and whether it is allocated (stored in the low bit) */
    size_t header;
    /**
     * We don't know what the size of the payload will be, so we will
     * declare it as a zero-length array.  This allow us to obtain a
     * pointer to the start of the payload.
     */
    uint8_t payload[];
} block_t;

/**
 * The first and last blocks on the heap. Both are NULL when the heap has
 * no blocks yet. Any code that adds, splits, or merges blocks must keep
 * mm_heap_last pointing at the block with the highest address.
 */
static block_t *mm_heap_first = NULL;
static block_t *mm_heap_last = NULL;

/** Rounds up `size` to the nearest multiple of `n` */
static size_t round_up(size_t size, size_t n) {
    return (size + (n - 1)) / n * n;
}

/**
 * Set's a block's header with the given size and allocation state.
 * `size` is the size of the whole block (header included) and must be a
 * multiple of ALIGNMENT, so its low bit is free to hold `is_allocated`.
 */
static void set_header(block_t *block, size_t size, bool is_allocated) {
    block->header = size | is_allocated;
}

/** Extracts a block's size (header included) from its header */
static size_t get_size(block_t *block) {
    return block->header & ~1;
}

/** Extracts a block's allocation state from its header */
static bool is_allocated(block_t *block) {
    return block->header & 1;
}

/**
 * Finds the first free block in the heap with at least the given size.
 * `size` is a whole-block size (header included).
 * If no block is large enough, returns NULL.
 */
static block_t *find_fit(size_t size) {
    // Traverse the blocks in the heap using the implicit list
    for (
        block_t *curr = mm_heap_first;
        mm_heap_last != NULL && curr <= mm_heap_last;
        curr = (void *) curr + get_size(curr)
    ) {
        // If the block is free and large enough for the allocation, return it
        if (!is_allocated(curr) && get_size(curr) >= size) {
            return curr;
        }
    }
    return NULL;
}

/**
 * Gets the header corresponding to a given payload pointer.
 * offsetof(block_t, payload) is the distance in bytes from the start of a
 * block_t to its payload field, which is 8 (the header).
 */
static block_t *block_from_payload(void *ptr) {
    return ptr - offsetof(block_t, payload);
}

/**
 * coalesce_free_blocks - Merges adjacent free blocks in the heap into
 *      single, larger free blocks. (Checkpoint 3)
 *
 * __attribute__((unused)) stops the compiler from warning that this
 * function is never called. You can remove it once you call it.
 */
__attribute__((unused))
static void coalesce_free_blocks(void) {
    
    if(mm_heap_first == NULL) {
        return;
    }

    block_t *curr = mm_heap_first;

    while ( curr <= mm_heap_last) {

        if (!is_allocated(curr)) {
            block_t *next = (void *) curr + get_size(curr);

            while (next <= mm_heap_last && !is_allocated(next)) {

                size_t bothSizes = get_size(curr) + get_size(next); //getting the current size with the next
                set_header(curr,bothSizes,false);

                if(next == mm_heap_last) {
                    mm_heap_last = curr;

                    break;
                }

                next = (void *) curr + get_size(curr);
            }
        }

        curr = (void *) curr + get_size(curr);
    }
}


/**
 * mm_init - Initializes the allocator state
 *
 * The driver calls mm_init before every trace, on a fresh, empty heap.
 * Reset ALL of your global variables here; values left over from the
 * previous trace point into a heap that no longer exists.
 *
 * Returns true on success, false if the heap could not be set up.
 */
bool mm_init(void) {
    // We want the first payload to start at ALIGNMENT bytes from the start of the heap
    void *padding = mem_sbrk(ALIGNMENT - sizeof(block_t));
    if (padding == (void *) -1) {
        return false;
    }

    // Initialize the heap with no blocks
    mm_heap_first = NULL;
    mm_heap_last = NULL;
    return true;
}

/**
 * mm_malloc - Allocates a block with the given size
 *
 * Returns a pointer to a payload of at least `size` bytes, aligned to 16
 * bytes. The block must lie inside the heap and must not overlap any other
 * allocated block. Returns NULL if the heap cannot grow.
 */

 /**
 * mm_malloc - Allocates a block with the given size
 *
 * Returns a pointer to a payload of at least `size` bytes, aligned to 16
 * bytes. The block must lie inside the heap and must not overlap any other
 * allocated block. Returns NULL if the heap cannot grow.
 */
void *mm_malloc(size_t size) {
    size = round_up(sizeof(block_t) + size, ALIGNMENT);

    // If there is a large enough free block, use it
    block_t *block = find_fit(size);
    if (block != NULL) {
        size_t ogsize = get_size(block);

        // Split the free block if the remaining space can hold another block
        if (ogsize - size >= ALIGNMENT) {
            block_t *remainder = (void *) block + size;
            set_header(block, size, true);
            set_header(remainder, ogsize - size, false);
        } else {
            set_header(block, ogsize, true);
        }

        return block->payload;
    }

    // Otherwise, a new block needs to be allocated at the end of the heap
    block = mem_sbrk(size);
    if (block == (void *) -1) {
        return NULL;
    }

    // Update mm_heap_first and mm_heap_last since we extended the heap
  
    if (mm_heap_first == NULL) {
        mm_heap_first = block;
    }
    mm_heap_last = block;

    // Initialize the block with the allocated size
    set_header(block, size, true);
    return block->payload;
}


/**
 * mm_free - Releases a block to be reused for future allocations
 *
 * `ptr` is a payload pointer returned by an earlier mm_malloc, mm_calloc,
 * or mm_realloc that has not been freed yet. mm_free(NULL) does nothing.
 */
void mm_free(void *ptr) {
    // mm_free(NULL) does nothing
    if (ptr == NULL) {
        return;
    }

    // Mark the block as unallocated
    block_t *block = block_from_payload(ptr);
    set_header(block, get_size(block), false);
}

/**
 * mm_realloc - Change the size of the block by mm_mallocing a new block,
 *      copying its data, and mm_freeing the old block. (Checkpoint 1)
 *
 * Returns a pointer to a payload of at least `size` bytes that holds the
 * same data as the old block.
 *
 * If old_ptr is NULL, behaves like mm_malloc(size).
 * If size is 0, behaves like mm_free(old_ptr) and returns NULL.
 */
void *mm_realloc(void *old_ptr, size_t size) {
    if (old_ptr == NULL) {
        return mm_malloc(size);
    }

    if (size == 0 ) {
        mm_free(old_ptr);
        return NULL;

    }

    block_t *oldBlock = block_from_payload(old_ptr);
    size_t oldSize = get_size(oldBlock) - sizeof(block_t);

    void *new_ptr = mm_malloc(size);
    if (new_ptr == NULL) {
        return NULL;
    }

    size_t copySize = oldSize < size ? oldSize : size;

    memcpy(new_ptr, old_ptr, copySize);

    mm_free(old_ptr);

    return new_ptr;

}


/**
 * mm_calloc - Allocate the block and set it to zero. (Checkpoint 1)
 *
 * Allocates an array of `nmemb` elements of `size` bytes each, with every
 * byte set to 0, and returns a pointer to it.
 */
void *mm_calloc(size_t nmemb, size_t size) {
    if (nmemb != 0 && size > SIZE_MAX / nmemb) {
        return NULL;
    }

    size_t totalSize = nmemb * size;

    void *ptr = mm_malloc(totalSize);
    if (ptr == NULL) {
        return NULL;
    }

    memset(ptr, 0, totalSize);
    return ptr;
}
/**
 * mm_checkheap - So simple, it doesn't need a checker!
 *
 * Optional, but strongly recommended. Running the driver with -D calls
 * this before every operation. It should print nothing while the heap is
 * fine, and print a message describing the problem as soon as it finds one.
 *
 * Ideas for what to check, walking every block from mm_heap_first:
 *   - Each block's size is a multiple of ALIGNMENT and at least ALIGNMENT.
 *   - Each payload address is a multiple of ALIGNMENT.
 *   - The walk lands exactly on mm_heap_last, and the last block ends
 *     exactly at mem_heap_hi() + 1.
 */
void mm_checkheap(void) {

}
