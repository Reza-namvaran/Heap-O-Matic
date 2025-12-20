#include <stdio.h>
#include <string.h>

#include "allocator.h"
#include "test_framework.h"

int test_heap_spray_prevention_addresses_vary() {
    hinit();

    const int num_allocs = 50;
    const size_t size = 128;
    void *ptrs[num_allocs];
    intptr_t addresses[num_allocs];

    for (int i = 0; i < num_allocs; i++) {
        ptrs[i] = halloc(size);
        TEST_ASSERT(ptrs[i] != NULL);
        addresses[i] = (intptr_t)ptrs[i];
    }

    int all_same = 1;
    for (int i = 1; i < num_allocs; i++) {
        if (addresses[i] != addresses[0]) {
            all_same = 0;
            break;
        }
    }
    TEST_ASSERT(all_same == 0);

    int unique_diffs = 0;
    intptr_t last_diff = 0;
    for (int i = 1; i < num_allocs; i++) {
        intptr_t diff = addresses[i] - addresses[i-1];
        if (i == 1 || diff != last_diff) {
            unique_diffs++;
            last_diff = diff;
        }
    }
    TEST_ASSERT(unique_diffs >= 10);

    for (int i = 0; i < num_allocs; i++) {
        hfree(ptrs[i]);
    }

    return 0;
}

int main() {
    TEST_RUN(test_heap_spray_prevention_addresses_vary);

    printf("All heap spraying prevention tests finished.\n");
    return 0;
}