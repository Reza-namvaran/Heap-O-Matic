#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>
#include <pthread.h>

typedef struct chunk_t {
    uint32_t size;
    uint8_t inuse;
    struct chunk_t *next;
};

typedef struct heap_t {
    struct chunk_t *start;
    size_t total;
    uint32_t avail;
    pthread_mutex_t lock;    
};

#endif