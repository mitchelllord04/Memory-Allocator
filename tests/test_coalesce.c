#include <stdio.h>
#include <string.h>
#include "../include/allocator.h"

int main() {
    printf("=== Coalescing demo ===\n");

    void *a = mallocate(32);
    void *b = mallocate(32);
    void *c = mallocate(32);

    memset(a, 0xA1, 32);
    memset(b, 0xB2, 32);
    memset(c, 0xC3, 32);

    printf("\nAfter allocating a,b,c:\n");
    print_blocks();

    mfree(b);
    printf("\nAfter freeing b (middle):\n");
    print_blocks();

    mfree(c);
    printf("\nAfter freeing c (b + c should coalesce):\n");
    print_blocks();

    mfree(a);
    printf("\nAfter freeing a (should coalesce into one free region):\n");
    print_blocks();

    return 0;
}