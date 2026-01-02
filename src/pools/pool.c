#include "pool.h"
#include <stdio.h>
#include <string.h>

extern const int pool_sizes[];

int pool_index(size_t size) {
    for (int i = 0; i < POOL_COUNT; i++) {
        if (size <= pool_sizes[i]) {
            return i;
        }
    }

    return -1;
}

void pool_hinit(void) {
    for (int i = 0; i < POOL_COUNT; i++) {
        GLOBAL_HEAP.pools[i].size = pool_sizes[i];
        GLOBAL_HEAP.pools[i].free = NULL;
        GLOBAL_HEAP.pools[i].total = 0;
    }
}

static void pool_refill(int idx) {
    // printf("In refill\n");
    pool_t *p = &GLOBAL_HEAP.pools[idx];

    size_t obj_size = sizeof(pool_hdr_t) + p->size;
    obj_size = align_up(obj_size, HEAP_ALIGNMENT);

    void *slab = halloc_raw(POOL_SLAB_SIZE);
    if (!slab) return;
    // printf("After slab check\n");

    uint8_t *curr = slab;
    uint8_t *end = curr + POOL_SLAB_SIZE;

    while (curr + obj_size <= end) {
        pool_hdr_t *hdr = (pool_hdr_t *)curr;
        hdr->pool_index = idx;

        pool_chunk_t *pc = (pool_chunk_t *)(curr + sizeof(pool_hdr_t));
        pc->next = p->free;
        p->free  = pc;

        curr += obj_size;
        p->total++;
    }
}

void *pool_halloc(size_t size) {
    int idx = pool_index(size);

    if (idx < 0) return NULL;
    pool_t *p = &GLOBAL_HEAP.pools[idx];

    if (!p->free) {
        pool_refill(idx);
        if (!p->free) {
            return NULL;
        }
    }
    pool_chunk_t *pc = p->free;
    p->free = pc->next;

    #if ENABLE_POISONING
        memset(pc, 0xAA, p->size);
    #endif

    return (void *)pc;
}

void pool_hfree(void *ptr) {
    if (!ptr) return;

    pool_hdr_t *hdr = (pool_hdr_t *)((uint8_t *)ptr - sizeof(pool_hdr_t));

    int idx = hdr->pool_index;
    pool_t *p = &GLOBAL_HEAP.pools[idx];

    pool_chunk_t *pc = (pool_chunk_t *)ptr;
    pc->next = p->free;
    p->free = pc;
}

