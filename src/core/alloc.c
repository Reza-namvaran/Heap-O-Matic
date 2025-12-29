#include "allocator.h"
#include "debug.h"
#include "split.h"
#include "quarantine.h"

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

    size_t payload = align_up(size, HEAP_ALIGNMENT);

    #if ENABLE_SPRAY_DETECT
    if (size > 32) {
        size_t extra_pad = 16 + (rand() & 0x3F);
        payload = align_up(payload + extra_pad, HEAP_ALIGNMENT);
    }
    #endif

    size_t total = payload + sizeof(chunk_t);

    chunk_t *curr = GLOBAL_HEAP.start;

    if (!curr) {
    #if ENABLE_QUARANTINE
        quarantine_flush_one();
        curr = GLOBAL_HEAP.start;
    #endif
    }

    while (curr) {

        if (!curr->inuse && curr->size >= payload) {
            curr = chunk_split(curr, payload);

            GLOBAL_HEAP.avail -= curr->size;

            curr->inuse = 1;

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
            return chunk_to_payload(curr);
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