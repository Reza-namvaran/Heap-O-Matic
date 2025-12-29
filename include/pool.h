#pragma once
#include "allocator.h"

typedef struct pool_chunk {
    struct pool_chunk *next;
} pool_chunk_t;

typedef struct pool {
    size_t size;
    pool_chunk_t *free;
    size_t total;
} pool_t;

void pool_hinit(void);
void *pool_halloc(size_t size);
void pool_hfree(chunk_t *c);