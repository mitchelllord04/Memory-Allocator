#include <stdio.h>
#include <string.h>
#include "../include/allocator.h"

int main() {
    printf("=== Splitting demo ===\n");

    void *big = mallocate(128);
    memset(big, 0xAA, 128);

    printf("\nAfter allocating big (128):\n");
    print_blocks();

    mfree(big);
    printf("\nAfter freeing big (should be one large free block):\n");
    print_blocks();

    void *small = mallocate(16);
    memset(small, 0xBB, 16);

    printf("\nAfter allocating small (16) into free block (should split):\n");
    print_blocks();

    mfree(small);
    return 0;
}