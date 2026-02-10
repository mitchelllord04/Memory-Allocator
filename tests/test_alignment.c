#include <stdio.h>
#include <stdint.h>
#include "../include/allocator.h"

static void row(const char *name, void *p) {
    printf("%-6s %p   mod16=%2lu   aligned=%s\n",
           name, p, (unsigned long)((uintptr_t)p % 16),
           is_aligned(p) ? "YES" : "NO");
}

int main() {
    printf("=== Alignment demo (16-byte) ===\n");
    void *p1 = mallocate(1);
    void *p2 = mallocate(7);
    void *p3 = mallocate(16);
    void *p4 = mallocate(31);
    void *p5 = mallocate(64);

    row("p1", p1);
    row("p2", p2);
    row("p3", p3);
    row("p4", p4);
    row("p5", p5);

    printf("\nBlock list:\n");
    print_blocks();

    mfree(p1); mfree(p2); mfree(p3); mfree(p4); mfree(p5);
    return 0;
}