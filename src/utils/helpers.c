#include "allocator.h"

static inline size_t align_up(size_t size, size_t align) {
    return (size + align - 1) & ~(align - 1);
}

static inline void *chunk_to_payload(chunk_t *c) {
    return (void *)(c + 1);
}

static inline chunk_t *payload_to_chunk(void *p) {
    return ((chunk_t *)p) - 1;
}