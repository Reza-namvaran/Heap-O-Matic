#include "allocator.h"
#include "test_framework.h"
#include "debug.h"

int test_hfree_null_does_nothing() {
    hinit();
    hfree(NULL); 
    TEST_ASSERT(heap_check() == 0);
    return 0;
}

int test_hfree_sets_inuse_zero() {
    hinit();
    void *p = halloc(64);
    chunk_t *c = payload_to_chunk(p);
    TEST_ASSERT(c->inuse == 1);

    hfree(p);
    TEST_ASSERT(c->inuse == 0);
    return 0;
}

int test_hfree_increases_avail() {
    hinit();
    void *p = halloc(128);
    size_t avail_before = GLOBAL_HEAP.avail;

    hfree(p);
    TEST_ASSERT(GLOBAL_HEAP.avail > avail_before);
    return 0;
}

#if ENABLE_POISONING
int test_hfree_poisons_payload() {
    hinit();
    void *p = halloc(32);
    unsigned char *payload = (unsigned char*)p;

    hfree(p);

    for (int i = 0; i < 32; i++) {
        TEST_ASSERT(payload[i] == 0xDD);
    }
    return 0;
}
#endif

int test_hfree_invalid_ptr_ignored() {
    hinit();
    void *invalid = (void*)0x1234; 
    hfree(invalid);
    TEST_ASSERT(heap_check() == 0);  
    return 0;
}

int main() {
    TEST_RUN(test_hfree_null_does_nothing);
    TEST_RUN(test_hfree_sets_inuse_zero);
    TEST_RUN(test_hfree_increases_avail);
    #if ENABLE_POISONING
    TEST_RUN(test_hfree_poisons_payload);
    #endif
    TEST_RUN(test_hfree_invalid_ptr_ignored);

    printf("All hfree() tests finished.\n");
    return 0;
}