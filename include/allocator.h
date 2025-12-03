#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>
#include <pthread.h>

#define HEAP_DEFUALT_SIZE (8 * 1024 * 1024)
#define HEAP_ALLIGNMENT 16

typedef struct chunk_t {
    uint32_t size;
    uint8_t inuse;
    struct chunk_t *next;
} chunk_t;

typedef struct heap_t {
    struct chunk_t *start;
    size_t total;
    uint32_t avail;
    pthread_mutex_t lock;    
} heap_t;

void hinit();
void *halloc(size_t size);
void hfree(void *ptr);

// Helpers exposed for tests and debug
extern heap_t GLOBAL_HEAP;

static inline void align_up(size_t size, size_t align);
static inline void *chunk_to_payload(chunk_t *c);
static inline chunk_t payload_to_chunk(void *p);

#endif