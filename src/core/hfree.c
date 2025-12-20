#include <string.h>
#include <errno.h>

#include "allocator.h"
#include "validate.h"
#include "coalescing.h"

void hfree(void *ptr) {
    if (!ptr) return; 

    pthread_mutex_lock(&GLOBAL_HEAP.lock);

    chunk_t *c = NULL;
    if (!validate_ptr(ptr, &c)) {
        pthread_mutex_unlock(&GLOBAL_HEAP.lock);
        return;
    }

    c->inuse = 0;
    GLOBAL_HEAP.avail += c->size;

    #if ENABLE_POISONING
        memset(chunk_to_payload(c), 0xDD, c->size);
    #endif

    coalesce(c);

    pthread_mutex_unlock(&GLOBAL_HEAP.lock);
}
