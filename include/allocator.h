#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>

#include "config.h"

/// TODO: remove if no padding considration
typedef enum {
    CHUNK_FREE,
    CHUNK_USED,
    CHUNK_GAURD
} chunk_kind_t;

typedef struct chunk_t {
    uint32_t size;
    uint32_t prev_size;
    // chunk_kind_t kind;
    uint8_t inuse;
    struct chunk_t *next;
} chunk_t;

typedef struct heap_t {
    struct chunk_t *start;  
    void *heap_lo;
    void *heap_hi;
    size_t total;
    uint32_t avail;
    pthread_mutex_t lock;    
} heap_t;

void hinit();
void *halloc(size_t size);
void hfree(void *ptr);

// Helpers exposed for tests and debug
extern heap_t GLOBAL_HEAP;

static inline size_t align_up(size_t size, size_t align) {
    return (size + align - 1) & ~(align - 1);
}

static inline __attribute__((always_inline)) 
void *chunk_to_payload(chunk_t *c) {
    return (void *)(c + 1);
}

static inline chunk_t *payload_to_chunk(void *p) {
    return ((chunk_t *)p) - 1;
}

static inline __attribute__((always_inline))
chunk_t *next_chunk(chunk_t *c) {
    chunk_t *next = (chunk_t *)((char *)c + sizeof(chunk_t) + c->size);
    return ((void *) next < GLOBAL_HEAP.heap_hi) ? next : NULL;
}

static inline __attribute__((always_inline))
chunk_t *prev_chunk(chunk_t *c) {
    if (c->prev_size == 0) {
        return NULL;
    }

    return (chunk_t *)((char *)c - sizeof(chunk_t) - c->prev_size);
}
#endif