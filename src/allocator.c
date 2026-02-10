#include "../include/allocator.h"
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

/*
 * Author: Mitchell Lord
 *
 * My custom implementation of malloc and free from stdlib.h.
 * Allocates memory by calling sbrk() to grow the heap.
 *
 * Uses a singly linked list of Block structs to track allocated and free memory blocks.
 * Each block contains metadata (size, status, next pointer) followed by useable memory.
 *
 * Free blocks are reused when possible, and contigous memory blocks are coalesced to
 * reduce fragmentation and manage memory more efficiently.
 */


/*
 * Represents a memory block in the allocator.
 * Each allocated block contains metadata followed by usable memory.
 *
 * Fields:
 *   size - number of bytes of usable memory in the block.
 *   free - 1 if the block is free, 0 if it is allocated.
 *   next - pointer to the next Block in the linked list.
 */
typedef struct __attribute__((aligned(16))) Block
{
    size_t size;
    int free;
    struct Block *next;
} Block;

// Memory blocks must be aligned to 16-byte boundaries
#define ALIGNMENT 16

// Size of metadata for each memory block, rounded up to the nearest multiple of ALIGNMENT
#define ALIGNED_METADATA_SIZE ((sizeof(Block) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

// Minimum size of a memory block: metadata + at least one aligned data unit
#define MIN_BLOCK_SIZE (ALIGNED_METADATA_SIZE + ALIGNMENT)

static size_t align(size_t size);
static Block *split(Block *block, size_t size);
static Block *coalesce(Block *block);

// Head of the linked list of Blocks used to track allocated memory.
static Block *head = NULL;

// Tracks the start of the memory allocated by sbrk().
// Used to check if pointers passed to mfree() are within the heap.
static void *heap_start = NULL;

/*
* Allocates a block of memory of 'size' bytes.
* The returned memory is aligned to ALIGNMENT bytes.
*
*
*
* Parameters:
*   size - the number of bytes requested by the user.
*
* Returns:
*   A pointer to the allocated memory,
*   or NULL if allocation fails.
*/
void *mallocate(size_t size)
{
    if (heap_start == NULL)
    {
        heap_start = sbrk(0);
    }

    // Ensure 16-byte alignment.
    size_t aligned_size = align(size);
    size_t total_size = ALIGNED_METADATA_SIZE + aligned_size;

    // If no memory has been allocated yet.
    if (head == NULL)
    {
        void *allocated = sbrk(total_size);
        if (allocated == (void *)-1)
        {
            return NULL;
        }

        Block *memory = (Block *)allocated;

        memory->size = aligned_size;
        memory->free = 0;
        memory->next = NULL;

        head = memory;

        return (void *)((char *)allocated + ALIGNED_METADATA_SIZE);
    }

    Block *curr = head;
    Block *last = NULL;

    // Traverse the linked list to find a free block large enough for the requested size.
    // If a suitable block is found, split it if it’s bigger than needed and return a pointer to its usable memory.
    while (curr != NULL)
    {
        if (curr->free && curr->size >= aligned_size)
        {
            curr = split(curr, aligned_size);
            return (void *)((char *)curr + ALIGNED_METADATA_SIZE);
        }

        last = curr;
        curr = curr->next;
    }

    // No suitable free block found: extend the heap and append a new block to the list.
    void *allocated = sbrk(total_size);
    if (allocated == (void *)-1)
    {
        return NULL;
    }

    Block *memory = (Block *)allocated;

    memory->size = aligned_size;
    memory->free = 0;
    memory->next = NULL;

    last->next = memory;

    return (void *)((char *)allocated + ALIGNED_METADATA_SIZE);
}

/*
 * Frees memory blocked pointed to by ptr.
 *
 * Ensures the passed in memory was allocated by this program
 * by checking that it is within the bounds of the heap and
 * that it is aligned to 16 bytes.
 *
 * Shrinks the heap using sbrk() if the block is at the end.
 *
 * Combines adjacent free blocks using coalesce() if possible.
 */
void mfree(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }

    // Verify that the memory is within the heap.
    void *heap_end = sbrk(0);
    if (ptr < heap_start || ptr >= heap_end)
    {
        return;
    }

    // Verify that the pointer is properly aligned.
    if (!is_aligned(ptr))
    {
        return;
    }

    // Set block status to free and coalesce.
    Block *metadata = (Block *)((char *)ptr - ALIGNED_METADATA_SIZE);
    if (metadata->free) return;
    metadata->free = 1;
    metadata = coalesce(metadata);
}

/*
 * Splits a memory block into two if it is larger than the requested size.
 *
 * Helps use memory more efficiently and reduces unnecessary calls to sbrk().
 * The original block keeps the requested size, and the remaining memory
 * becomes a new free block linked into the list.
 */
static Block *split(Block *block, size_t size)
{
    // If the block is too small to split.
    // Mark as allocated and return it.
    if (block->size < size + MIN_BLOCK_SIZE)
    {
        block->free = 0;
        return block;
    }

    // Calculate the size of the remaining memory after splitting.
    size_t leftover_size = block->size - size - ALIGNED_METADATA_SIZE;

    // Create a new free block from the leftover memory.
    Block *new_block = (Block *)((char *)block + ALIGNED_METADATA_SIZE + size);
    new_block->size = leftover_size;
    new_block->free = 1;
    new_block->next = block->next;

    // Update the original block to the requested size and link it to the new free block.
    block->size = size;
    block->free = 0;
    block->next = new_block;

    return block;
}

/*
 * Merges the given free block with adjacent blocks of free memory, if any,
 * to reduce fragmentation and utilize memory more efficiently.
 */
static Block *coalesce(Block *block)
{
    // Merge the block with consecutive free blocks that follow it in memory
    while (block->next && block->next->free)
    {
        block->size += ALIGNED_METADATA_SIZE + block->next->size;
        block->next = block->next->next;
    }

    // Iterate to the preceding block.
    Block *curr = head;
    Block *prev = NULL;
    while (curr && curr != block)
    {
        prev = curr;
        curr = curr->next;
    }

    // If the previous block is free, merge it with the current block and return it.
    if (prev && prev->free)
    {
        prev->size += ALIGNED_METADATA_SIZE + block->size;
        prev->next = block->next;
        return prev;
    }

    return block;
}

/*
 * Aligns the given size to the next multiple of ALIGNMENT (16 bytes).
 * Uses bitwise arithmetic to round up efficiently.
 */
static size_t align(size_t size)
{
    return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1); 
}

/*
 * Prints the list of blocks.
 * Used for debugging and testing purposes.
 */
void print_blocks()
{
    Block *curr = head;
    printf("Blocks list:\n");
    while (curr != NULL)
    {
        printf("  Block at %p: size=%zu, free=%s, next=%p\n",
               curr, curr->size, curr->free ? "true" : "false", curr->next);
        curr = curr->next;
    }
}

// Helper function to check alignment
int is_aligned(void *ptr) {
    return ((uintptr_t)ptr % ALIGNMENT) == 0;
}



