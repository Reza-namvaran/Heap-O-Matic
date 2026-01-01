#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "allocator.h"
#include "quarantine.h"

heap_t GLOBAL_HEAP;

void hinit() {
    void *heap_base = mmap(NULL, HEAP_DEFAULT_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (heap_base == MAP_FAILED) {
        errno = ENOMEM;
        return;
    }

    // Defensive Programming: Align to start
    size_t pad = align_offset(heap_base);

    /// TODO: Decide on final design
    // for now we assume mmap is always aligned
    if (pad != 0) {
        printf("pad is not 0: %i", pad);
        pad = 0;
    }
    
    chunk_t *aligned_start = (chunk_t *)((char *)heap_base + pad);

    // If padded, make a dummy in-use chunk for the padding space
    if (pad > 0) {
        chunk_t *dummy = (chunk_t *) heap_base;
        dummy->size = pad;
        dummy->inuse = 1;
        dummy->next = aligned_start;
    }

    uintptr_t mix = (uintptr_t)heap_base ^ (uintptr_t)&GLOBAL_HEAP;
    mix ^= (uintptr_t)time(NULL);
    GLOBAL_HEAP.cookie = (uint32_t)(mix ^ (mix >> 32));

    // Init first free chunk (rest of heap)
    aligned_start->magic = make_magic(aligned_start);
    aligned_start->prev_size = 0;
    aligned_start->size  = HEAP_DEFAULT_SIZE - pad - sizeof(chunk_t);
    aligned_start->inuse = 0;
    aligned_start->next  = NULL;

    // Set global heap
    GLOBAL_HEAP.start   = (pad > 0) ? (chunk_t *)heap_base : aligned_start;
    GLOBAL_HEAP.heap_lo = heap_base;
    GLOBAL_HEAP.heap_hi = (char *)heap_base + HEAP_DEFAULT_SIZE;
    GLOBAL_HEAP.total   = HEAP_DEFAULT_SIZE;
    GLOBAL_HEAP.avail   = aligned_start->size;
    pthread_mutex_init(&GLOBAL_HEAP.lock, NULL); // Thread Safe

    #if ENABLE_POISONING
        // Fill free payload with poison pattern (0xDEADBEEF)
        void *payload = chunk_to_payload(aligned_start);
        memset(payload, 0xDB, aligned_start->size);
    #endif

    #if ENABLE_QUARANTINE
        quarantine_init();
    #endif
}