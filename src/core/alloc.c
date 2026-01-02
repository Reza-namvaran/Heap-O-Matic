#include "allocator.h"
#include "debug.h"
#include "split.h"
#include "quarantine.h"
#include "pool.h"

#include <string.h>
#include <errno.h>
#include <stdio.h> 

void *halloc_raw(size_t size) {
    if (size == 0) return NULL;

    size_t payload = align_up(size, HEAP_ALIGNMENT);

    #if ENABLE_SPRAY_DETECT
        if (size > 32) {
            size_t extra_pad = 16 + (rand() & 0x3F);
            payload = align_up(payload + extra_pad, HEAP_ALIGNMENT);
        }
    #endif

    chunk_t *curr = GLOBAL_HEAP.start;

    #if ENABLE_QUARANTINE
        if (!curr) {
            quarantine_flush_one();
            curr = GLOBAL_HEAP.start;
        }
    #endif

    while (curr) {
        if (!curr->inuse && curr->size >= payload) {
            curr = chunk_split(curr, payload);

            GLOBAL_HEAP.avail -= curr->size;
            curr->inuse = 1;

            #if ENABLE_POISONING
                    memset(chunk_to_payload(curr), 0xAA, curr->size);
            #endif
                    return chunk_to_payload(curr);
        }
        curr = curr->next;
    }

    errno = ENOMEM;
    return NULL;
}


void *halloc(size_t size) {
    pthread_mutex_lock(&GLOBAL_HEAP.lock);

    #if ENABLE_DEBUG
        if (heap_check() != 0) {
            heap_dump();
            abort();
        }
    #endif

    /* ---------- POOL FAST PATH ---------- */
    void *p = pool_halloc(size);
    if (p) {
        pthread_mutex_unlock(&GLOBAL_HEAP.lock);
        return p;
    }

    /* ---------- HEAP PATH ---------- */
    void *res = halloc_raw(size);

    #if ENABLE_DEBUG
        if (heap_check() != 0) {
            heap_dump();
            abort();
        }
    #endif

    pthread_mutex_unlock(&GLOBAL_HEAP.lock);
    return res;
}
