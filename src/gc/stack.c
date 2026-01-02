#include "gc.h"
#include "allocator.h"
#include "validate.h"
#include "quarantine.h"
#include "coalescing.h"

#include <pthread.h>
#include <stdint.h>
#include <string.h>

static int gc_enabled = 1;

// Stack boundaries (Linux / POSIX safe)
static inline void* *stack_top(void) {
    volatile int x;
    return (void *)&x;
}

static inline void *stack_bottem(void) {
    pthread_attr_t attr;
    pthread_getattr_np(pthread_self(), &attr);

    void *base;
    size_t size;
    pthread_attr_getstack(&attr, &base, &size);

    pthread_attr_destroy(&attr);
    return (void *)base + size;
}