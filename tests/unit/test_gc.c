#include "allocator.h"
#include "coalescing.h"
#include "split.h"
#include "hgc.h"
#include "debug.h"
#include "test_framework.h"

#include <stdio.h>

static void allocate_and_drop(void) {
    void *x = halloc(512);
    (void)x;
}

int test_gc_frees_unreachable() {
    hinit();
    allocate_and_drop();
    gc_collect();

    TEST_ASSERT(GLOBAL_HEAP.avail > 512);
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

    printf("All hinit() tests finished.\n");
    return 0;
}

