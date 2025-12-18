#include "debug.h"

int heap_check(void) {
    chunk_t *curr = GLOBAL_HEAP.start;
    size_t free_sum = 0;

    while (curr) {
        if (!is_aligned(chunk_to_payload(curr))) {
            return -1;
        }

        if (curr->size == 0) {
            return -2;
        }

        if (!curr->inuse) {
            free_sum += curr->size;
        }

        curr = curr->next;
    }

    return (free_sum == GLOBAL_HEAP.avail) ? 0 : -3;
}

void heap_dump() {
    chunk_t *curr = GLOBAL_HEAP.start;
    int i = 0;

    printf("----- Heap Dump -----\n");

    while(curr) {
        void *start   = (void *)curr;
        void *payload = chunk_to_payload(curr);
        void *end     = (char *)payload + curr->size;

        printf("[%02d] chunk=%p payload=%p end=%p | size=%u | %s\n", i++, start, payload, end, curr->size, curr->inuse ? "USED" : "FREE");

        if (curr->next) {
            void *expected = (char *)payload + curr->size;
            if (expected != (void *)curr->next) {
                printf("WARNING! Gap or Overlap detected\n");
            }
        }

        curr = curr->next;
    }

    printf("TOTAL: %zu bytes | FREE: %u bytes\n", GLOBAL_HEAP.total, GLOBAL_HEAP.avail);

    printf("---- End of Dump ----\n");

}