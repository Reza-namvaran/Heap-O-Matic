#include "allocator.h"
#include "debug.h"
#include "split.h"

#include <string.h>
#include <errno.h>

void *halloc(size_t size) {
    if (size == 0) return NULL;

    pthread_mutex_lock(&GLOBAL_HEAP.lock);

    #ifdef ENABLE_DEBUG
        if (heap_check() != 0) {
            heap_dump();
            abort();
        }
    #endif

    size_t pad = 0;
    size_t payload = align_up(size, HEAP_ALIGNMENT);
    #if ENABLE_SPRAY_DETECT
    if (size > 32) {
        pad = GLOBAL_HEAP.cookie & 0xF;
        if (pad == 0) pad = 8;
        payload += pad;
    }
    #endif

    size_t total = payload + sizeof(chunk_t);

    chunk_t *curr = GLOBAL_HEAP.start;

    while (curr) {
        if (!curr->inuse && curr->size >= payload) {
            // Split if enough space remains
            curr = chunk_split(curr, payload);

            curr->inuse = 1;
            GLOBAL_HEAP.avail -= curr->size;

            #ifdef ENABLE_DEBUG
                if (heap_check() != 0) {
                    heap_dump();
                    abort();
                }
            #endif

            #if ENABLE_POISONING
                memset(chunk_to_payload(curr), 0xAA, curr->size);
            #endif

            pthread_mutex_unlock(&GLOBAL_HEAP.lock);
            #if ENABLE_SPRAY_DETECT
                return (char *)chunk_to_payload(curr) + pad;
            #else
                return chunk_to_payload(curr);
            #endif
        }

        curr = curr->next;
    }

    #ifdef ENABLE_DEBUG
        if (heap_check() != 0) {
            heap_dump();
            abort();
        }
    #endif

    pthread_mutex_unlock(&GLOBAL_HEAP.lock);
    errno = ENOMEM;
    return NULL;
}