#include "validate.h"

int validate_ptr(void *ptr, chunk_t **out) {
    if (!ptr) {
        return invalid_ptr();
    }

    if (ptr < GLOBAL_HEAP.heap_lo || ptr >= GLOBAL_HEAP.heap_hi) {
        return invalid_ptr();
    }

    if (!is_aligned(ptr)) {
        return invalid_ptr();
    }

    chunk_t *c = payload_to_chunk(ptr);

    if ((void *)c < GLOBAL_HEAP.heap_lo || (void *)c >= GLOBAL_HEAP.heap_hi) {
        return invalid_ptr();
    }

    if (c->size == 0 || c->size > GLOBAL_HEAP.total) {
        return invalid_ptr();
    }

    if (!c->inuse) {
        return invalid_ptr();
    }

    if (c->magic != (CHUNK_MAGIC ^ GLOBAL_HEAP.cookie) || c->tail_magic != (CHUNK_MAGIC ^ GLOBAL_HEAP.cookie)) {
        return invalid_ptr();
    }

    *out = c;
    return 1;
}