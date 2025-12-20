#include "coalescing.h"

void coalesce(chunk_t *c) {
    // Coalesce with next
    chunk_t *next = next_chunk(c);
    if (next && !next->inuse) {
        c->size += sizeof(chunk_t) + next->size;
        c->next = next->next;
        GLOBAL_HEAP.avail += sizeof(chunk_t);

        if (c->next) {
            c->next->prev_size = c->size;
        }
    }

    chunk_t *prev = prev_chunk(c);
    if (prev && prev->inuse) {
        prev->size += sizeof(chunk_t) + c->size;
        prev->next = c->next;
        GLOBAL_HEAP.avail += sizeof(chunk_t);

        if (prev->next) {
            prev->next->prev_size = prev->size;
        }
    }
}