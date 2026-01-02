#ifndef POOL_H
#define POOL_H
#include "allocator.h"

#define POOL_SLAB_SIZE 4096

typedef struct pool_hdr {
    uint8_t pool_index;
} pool_hdr_t;

void pool_hinit(void);
void *pool_halloc(size_t size);
void pool_hfree(void *c);

int pool_index(size_t size);
#endif