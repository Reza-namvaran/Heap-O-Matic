#include "hgc.h"
#include "allocator.h"
#include "validate.h"
#include "quarantine.h"
#include "coalescing.h"

#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

static int gc_enabled = 1;

// Stack boundaries (Linux / POSIX safe)

static inline void *stack_top(void) {
    volatile int x;
    return (void *)&x;
}

static inline void *stack_bottom(void) {
    pthread_attr_t attr;
    pthread_getattr_np(pthread_self(), &attr);

    void *base;
    size_t size;
    pthread_attr_getstack(&attr, &base, &size);

    pthread_attr_destroy(&attr);
    return (char *)base + size;
}

//Mark phase
static void gc_mark_ptr(void *p) {
    if (p < GLOBAL_HEAP.heap_lo || p >= GLOBAL_HEAP.heap_hi)
        return;

    if (!is_aligned(p))
        return;

    chunk_t *c = payload_to_chunk(p);

    /* ignore pool objects */
    if (c->size == 0 || c->size > GLOBAL_HEAP.total)
        return;

    if ((void *)c < GLOBAL_HEAP.heap_lo || (void *)c >= GLOBAL_HEAP.heap_hi)
        return;

    if (!c->inuse)
        return;

    if (c->magic != make_magic(c))
        return;

    if (c->marked)
        return;

    c->marked = 1;


    /* Conservative scan payload for interior pointers */
    uintptr_t *data = (uintptr_t *)chunk_to_payload(c);
    size_t words = c->size / sizeof(uintptr_t);

    for (size_t i = 0; i < words; i++) {
        gc_mark_ptr((void *)data[i]);
    }
}

static inline uintptr_t *current_sp(void) {
    volatile uintptr_t x;
    return (uintptr_t *)&x;
}

static inline void get_stack_bounds(uintptr_t **low, uintptr_t **high) {
    pthread_attr_t attr;
    pthread_getattr_np(pthread_self(), &attr);

    void *base;
    size_t size;
    pthread_attr_getstack(&attr, &base, &size);

    pthread_attr_destroy(&attr);

    *low  = (uintptr_t *)base;
    *high = (uintptr_t *)((char *)base + size);
}


static void gc_mark_stack(void) {
    uintptr_t *sp = current_sp();
    uintptr_t *stack_low, *stack_high;

    get_stack_bounds(&stack_low, &stack_high);

    /* stack grows downward on Linux */
    uintptr_t *p = sp;

    if (p < stack_low)
        p = stack_low;

    for (; p < stack_high; p++) {
        gc_mark_ptr((void *)*p);
    }
}

// Sweep phase
static void gc_sweep(void) {
    chunk_t *c = GLOBAL_HEAP.start;

    while (c) {
        if (c->inuse) {
            if (!c->marked) {
                /* unreachable */
                c->inuse = 0;

                #if ENABLE_POISONING
                    memset(chunk_to_payload(c), 0xDD, c->size);
                #endif

                #if ENABLE_QUARANTINE
                    quarantine_push(c);
                #else
                    coalesce(c);
                #endif
            } else {
                /* reachable */
                c->marked = 0;
            }
        }
        c = c->next;
    }
}

// Public API
void gc_collect_locked(void) {
    gc_mark_stack();
    gc_sweep();
}

void gc_collect(void) {
    if (!gc_enabled) return;

    pthread_mutex_lock(&GLOBAL_HEAP.lock);
    gc_collect_locked();
    pthread_mutex_unlock(&GLOBAL_HEAP.lock);
}

void gc_enable(int on) {
    gc_enabled = on;
}
