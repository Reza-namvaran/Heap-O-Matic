#include <string.h>

#include "allocator.h"
#include "test_framework.h"

int test_halloc_returns_nonnull() {
    hinit();
    void *ptr = halloc(64);
    TEST_ASSERT(ptr != NULL);
    return 0;
}

int test_halloc_multiple_allocations() {
    hinit();
    void *p1 = halloc(512);
    void *p2 = halloc(1024);
    TEST_ASSERT(p1 != NULL);
    TEST_ASSERT(p2 != NULL);
    TEST_ASSERT(p1 != p2); // distinct pointers
    return 0;
}

int test_halloc_multiple_pool_allocations() {
    hinit();
    void *p1 = halloc(64);
    void *p2 = halloc(32);
    TEST_ASSERT(p1 != NULL);
    TEST_ASSERT(p2 != NULL);
    TEST_ASSERT(p1 != p2); // distinct pointers
    return 0;
}

int test_halloc_avail_decreases() {
    hinit();
    size_t before = GLOBAL_HEAP.avail;
    void *p = halloc(128);
    TEST_ASSERT(GLOBAL_HEAP.avail < before);
    return 0;
}

int test_halloc_splits_chunk() {
    hinit();
    chunk_t *first = GLOBAL_HEAP.start;
    size_t org_size = first->size;
    void *ptr = halloc(512);
    TEST_ASSERT(first->size == 512);
    TEST_ASSERT(first->next != NULL);
    TEST_ASSERT(first->next->size == org_size - 512 - sizeof(chunk_t));
    TEST_ASSERT(first->next->inuse == 0);
    return 0;
}

#if ENABLE_POISONING
int test_halloc_poison_payload() {
    hinit();
    void *ptr = halloc(16);
    unsigned char *payload = (unsigned char*)ptr;
    for (int i = 0; i < 16; i++) {
        TEST_ASSERT(payload[i] == 0xAA); // matches halloc poisoning
    }

    return 0;
}
#endif

int main() {
    TEST_RUN(test_halloc_returns_nonnull);
    TEST_RUN(test_halloc_multiple_allocations);
    TEST_RUN(test_halloc_multiple_pool_allocations);
    TEST_RUN(test_halloc_avail_decreases);
    TEST_RUN(test_halloc_splits_chunk);
    #if ENABLE_POISONING
    TEST_RUN(test_halloc_poison_payload);
    #endif

    printf("All halloc() tests finished.\n");
    return 0;
}