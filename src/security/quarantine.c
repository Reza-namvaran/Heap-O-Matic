#include "quarantine.h"
#include "coalescing.h"

#include <string.h>

static chunk_t *q[QUARANTINE_MAX];
static size_t q_len = 0;

void quarantine_init(void) {
    q_len = 0;
}

void quarantine_push(chunk_t *c) {
    if (q_len < QUARANTINE_MAX) {
        q[q_len++] = c;
        return;
    }

    quarantine_flush_one();
    q[q_len++] = c;
}

void quarantine_flush_one(void) {
    chunk_t *old = q[0];

    memmove(&q[0], &q[1], sizeof(chunk_t *) * (QUARANTINE_MAX - 1));
    q_len--;

    coalesce(old);
}

void quarantine_flush_all(void) {
    while (q_len > 0) {
        quarantine_flush_one();
    }
}

int get_quarantine_size() {
    return q_len;
}