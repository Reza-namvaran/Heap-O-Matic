#ifndef ALLOCATOR_H
#define ALLOCATOR_H
#include <stdlib.h>

typedef struct block {
    size_t size;
    int free;
    struct block *next;
} block_t;

void* mem_alloc(size_t size);
void mem_free(void *ptr);

#endif