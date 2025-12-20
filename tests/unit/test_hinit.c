#include "allocator.h"
#include "test_framework.h"

int test_hinit_start_nonnull() {
    hinit();
    TEST_ASSERT(GLOBAL_HEAP.start != NULL);
    return 0;
}

int test_hinit_total_size() {
    hinit();
    TEST_ASSERT(GLOBAL_HEAP.total == HEAP_DEFAULT_SIZE);
    return 0;
}

int test_hinit_avail_matches_first_chunck() {
    hinit();
    chunk_t *first = GLOBAL_HEAP.start;
    if (first->inuse == 1) first = first->next;
    TEST_ASSERT(GLOBAL_HEAP.avail == first->size);
    return 0; 
}

int test_hinit_first_chunk_free() {
    hinit();
    chunk_t *first = GLOBAL_HEAP.start;
    if (first->inuse == 1) first = first->next;
    TEST_ASSERT(first->inuse == 0);
    return 0; 
}

int main() {
    TEST_RUN(test_hinit_start_nonnull);
    TEST_RUN(test_hinit_total_size);
    TEST_RUN(test_hinit_avail_matches_first_chunck);
    TEST_RUN(test_hinit_first_chunk_free);

    printf("All hinit() tests finished.\n");
    return 0;
}

