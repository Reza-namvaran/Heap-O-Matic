#include "allocator.h"
#include "coalescing.h"
#include "quarantine.h"
#include "split.h"
#include "hgc.h"
#include "pool.h"
#include "debug.h"
#include "test_framework.h"

#include <stdio.h>

static void allocate_and_drop(void) {
    void *x = halloc(512);
    asm volatile("" : : "g"(x) : "memory");
}

static void allocate_multiple(void **ptrs, size_t n) {
    for (size_t i = 0; i < n; i++) {
        ptrs[i] = halloc(512);
        asm volatile("" : : "g"(ptrs[i]) : "memory");
    }
}

int test_gc_frees_unreachable() {
    hinit();
    uint32_t org = GLOBAL_HEAP.avail;
    allocate_and_drop();
    uint32_t after_allocation = GLOBAL_HEAP.avail;
    gc_collect();
    uint32_t after_gc = GLOBAL_HEAP.avail;

    TEST_ASSERT(org > after_allocation);
    #if ENABLE_QUARANTINE
        TEST_ASSERT(get_quarantine_size() > 0);
    #else
        TEST_ASSERT(org == after_gc);
    #endif
    return 0;
}


int test_gc_reclaims_multiple() {
    hinit();
    void *ptrs[5];
    // printf("\n%i\n", GLOBAL_HEAP.avail);
    allocate_multiple(ptrs, 5);

    uint32_t before_gc = GLOBAL_HEAP.avail;
    // printf("\n%i\n", before_gc);

    // drop all references
    for (int i = 0; i < 5; i++) {
        ptrs[i] = NULL;
    }
    
    /* prevent compiler keeping old values */
    asm volatile("" ::: "memory");

    gc_collect();
    uint32_t after_gc = GLOBAL_HEAP.avail;
    // printf("%i\n", after_gc);

    #if ENABLE_QUARANTINE
        TEST_ASSERT(get_quarantine_size() >= 4);
    #else
        TEST_ASSERT(after_gc > before_gc);
    #endif
    return 0;
}

int test_gc_handles_interior_pointer() {
    hinit();
    void *a = halloc(512);

    // simulate interior pointer
    void *interior = (char *)a + 128;

    asm volatile("" : : "g"(interior) : "memory");

    gc_collect();
    chunk_t *c = payload_to_chunk(a);
    TEST_ASSERT(c->inuse == 1); // GC sees chunk as reachable from interior pointer
    return 0;
}

int test_gc_keeps_reachable() {
    hinit();

    void *a = halloc(512);
    gc_collect();

    chunk_t *c = payload_to_chunk(a);
    TEST_ASSERT(c->inuse == 1);
    return 0;
}


int main() {
    TEST_RUN(test_gc_frees_unreachable);
    TEST_RUN(test_gc_keeps_reachable);
    TEST_RUN(test_gc_reclaims_multiple);
    TEST_RUN(test_gc_handles_interior_pointer);

    printf("All gc tests finished.\n");
    return 0;
}

