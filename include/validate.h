#pragma once
#include <errno.h>
#include "allocator.h"

static inline __attribute__((always_inline))
int invalid_ptr(void) {
    errno = EINVAL;
    return 0;
}

int valiate_ptr(void *ptr, chunk_t **out);