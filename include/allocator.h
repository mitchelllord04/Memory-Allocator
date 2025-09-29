#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h> // for size_t

void *mallocate(size_t size);
void mfree(void *ptr);
int is_aligned(void *ptr);
void print_blocks(void);

#endif
