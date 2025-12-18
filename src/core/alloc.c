#include "allocator.h"
#include <string.h>
#include <errno.h>

void *halloc(size_t size) {
    if (size == 0) return NULL;

    pthread_mutex_lock(&GLOBAL_HEAP.lock);

    size_t payload = align_up(size, HEAP_ALIGNMENT);
    size_t total = payload + sizeof(chunk_t);

    chunk_t *curr = GLOBAL_HEAP.start;

    while (curr) {
        if (!curr->inuse && curr->size >= payload) {
            // Split if enough space remains
            if (curr->size >= payload + sizeof(chunk_t) + HEAP_ALIGNMENT) {
                chunk_t *rem = (chunk_t *)((char *)curr + sizeof(chunk_t) + payload);
                rem->size    = curr->size - payload - sizeof(chunk_t);
                rem->inuse   = 0;
                rem->next    = curr->next;

                curr->next = rem;
                curr->size = payload;
            }

            curr->inuse = 1;
            GLOBAL_HEAP.avail -= curr->size;

            #if ENABLE_POISONING
                memset(chunk_to_payload(curr), 0xAA, curr->size);
            #endif

            pthread_mutex_unlock(&GLOBAL_HEAP.lock);
            return chunk_to_payload(curr);
        }

        curr = curr->next;
    }

    pthread_mutex_unlock(&GLOBAL_HEAP.lock);
    errno = ENOMEM;
    return NULL;
}