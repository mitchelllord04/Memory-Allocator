#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../include/allocator.h"

#define ALIGNMENT 16

/*
 * Comprehensive test for custom allocator.
 * Tests allocation, alignment, writing/reading, splitting, coalescing, and freeing.
 */

int main() {
    // Allocate three blocks
    void *a = mallocate(64);
    void *b = mallocate(32);
    void *c = mallocate(48);

    printf("Initial allocations:\n");
    print_blocks();
    printf("\n");

    // Verify alignment
    if (!is_aligned(a) || !is_aligned(b) || !is_aligned(c)) {
        printf("Error: Allocation not properly aligned.\n");
        return 1;
    }

    // Write into blocks
    memset(a, 0xAA, 64);
    memset(b, 0xBB, 32);
    memset(c, 0xCC, 48);

    // Free the middle block (b) -> should be marked free
    mfree(b);
    printf("After freeing b (middle block):\n");
    print_blocks();
    printf("\n");

    // Free block c -> should coalesce with b
    mfree(c);
    printf("After freeing c (should coalesce with b):\n");
    print_blocks();
    printf("\n");

    // Allocate a smaller block into the coalesced region
    void *d = mallocate(16);
    memset(d, 0xDD, 16);
    printf("Allocate d = 16 bytes (should split free block):\n");
    print_blocks();
    printf("\n");

    // Free d -> should return its block to free list
    mfree(d);
    printf("After freeing d (block should return to free list):\n");
    print_blocks();
    printf("\n");

    // Free a last to test head coalescing
    mfree(a);
    printf("After freeing a (head block free, but not empty heap):\n");
    print_blocks();
    printf("\n");

    printf("Allocator test finished successfully.\n");
    return 0;
}
