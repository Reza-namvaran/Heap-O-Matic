#include "allocator.h"
#include <string.h>
#include <errno.h>

void hfree(void *ptr) {
    if (!ptr) return; 

    pthread_mutex_lock(&GLOBAL_HEAP.lock);

    chunk_t *c = payload_to_chunk(ptr);
    chunk_t *curr = GLOBAL_HEAP.start;

    int found = 0;
    while (curr) {
        if (curr == c) {
            found = 1;
            break;
        }
        curr = curr->next;
    }

    if (!found) {
        errno = EINVAL;
        pthread_mutex_unlock(&GLOBAL_HEAP.lock);
        return;
    }

    if (!c->inuse) {
        errno = EINVAL; 
        pthread_mutex_unlock(&GLOBAL_HEAP.lock);
        return;
    }

    c->inuse = 0;
    GLOBAL_HEAP.avail += c->size;

    #if ENABLE_POISONING
        memset(chunk_to_payload(c), 0xDD, c->size);
    #endif

    if (c->next && !c->next->inuse) {
        c->size += sizeof(chunk_t) + c->next->size;
        c->next = c->next->next;
    }

    curr = GLOBAL_HEAP.start;
    while (curr && curr->next) {
        if (curr->next == c && !curr->inuse) {
            curr->size += sizeof(chunk_t) + c->size;
            curr->next = c->next;
            break;
        }
        curr = curr->next;
    }

    pthread_mutex_unlock(&GLOBAL_HEAP.lock);
}
