#include "split.h"

chunk_t *chunk_split(chunk_t *chunk, size_t payload) {
    // Not enough room for a split
    if (chunk->size < payload + sizeof(chunk_t) + HEAP_ALIGNMENT) {
        return chunk;
    }

    chunk_t *rem = (chunk_t *)((char *)chunk + sizeof(chunk_t) + payload);

    rem->prev_size = payload;
    rem->size  = chunk->size - sizeof(chunk_t) - payload;
    rem->inuse = 0;
    rem->next  = chunk->next;
    rem->magic = CHUNK_MAGIC ^ GLOBAL_HEAP.cookie;
    rem->tail_magic = CHUNK_MAGIC ^ GLOBAL_HEAP.cookie;


    chunk->size  = payload;
    chunk->next  = rem;
    chunk->magic = CHUNK_MAGIC ^ GLOBAL_HEAP.cookie;
    chunk->tail_magic = CHUNK_MAGIC ^ GLOBAL_HEAP.cookie;

    if (rem->next) {
        rem->next->prev_size = rem->size;
    }

    GLOBAL_HEAP.avail -= sizeof(chunk_t);
    return chunk;
}