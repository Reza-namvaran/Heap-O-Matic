#include "debug.h"

#include <stdio.h>

int heap_check(void) {
    chunk_t *curr = GLOBAL_HEAP.start;
    size_t free_sum = 0;

    while (curr) {
        if (!is_aligned(chunk_to_payload(curr))) {
            printf("\nAlign Error\n");
            return -1;
        }

        if (curr->size == 0) {
            printf("\nSize is 0\n");
            return -2;
        }

        if (!curr->inuse) {
            free_sum += curr->size;
        }

        if (curr->magic != (CHUNK_MAGIC ^ GLOBAL_HEAP.cookie) || curr->tail_magic != (CHUNK_MAGIC ^ GLOBAL_HEAP.cookie)) {
            printf("\nHeap metadata corruption detected.\n");
            printf("%i\n", CHUNK_MAGIC);
            printf("%i\n", GLOBAL_HEAP.cookie);
            printf("%i\n", CHUNK_MAGIC ^ GLOBAL_HEAP.cookie);
            return -4;
        }

        curr = curr->next;
    }

    if (free_sum != GLOBAL_HEAP.avail) {
            printf("free sum mismatch: %zu, avail: %u\n", free_sum, GLOBAL_HEAP.avail);
            return -3;
    }

    return 0;
}

void heap_dump() {
    chunk_t *curr = GLOBAL_HEAP.start;
    int i = 0;

    printf("\n----- Heap Dump -----\n");

    while(curr) {
        void *start   = (void *)curr;
        void *payload = chunk_to_payload(curr);
        void *end     = (char *)payload + curr->size;

        printf("[%02d] chunk=%p payload=%p end=%p | size=%u | %s\n", i++, start, payload, end, curr->size, curr->inuse ? "USED" : "FREE");

        if (curr->next) {
            void *expected = (char *)curr + curr->size + sizeof(chunk_t);
            if (expected != (void *)curr->next) {
                printf("WARNING! Gap or Overlap detected\n");
            }
        }

        curr = curr->next;
    }

    printf("TOTAL: %zu bytes | FREE: %u bytes\n", GLOBAL_HEAP.total, GLOBAL_HEAP.avail);

    printf("---- End of Dump ----\n");

}