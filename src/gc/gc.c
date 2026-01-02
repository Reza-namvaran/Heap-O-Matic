#include "hgc.h"
#include "allocator.h"
#include "validate.h"
#include "quarantine.h"
#include "coalescing.h"

#include <pthread.h>
#include <stdint.h>
#include <string.h>

static int gc_enabled = 1;

static void gc_mark_ptr(void *p);

// Roots: Stack and ELF
static inline uintptr_t current_sp(void) {
    volatile uintptr_t x;
    return (uintptr_t)&x;
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

/* Provided by linker on Linux */
extern char __data_start[];
extern char _edata[];
extern char __bss_start[];
extern char _end[];

static void gc_mark_globals(void) {
    uintptr_t *p = (uintptr_t *)__data_start;
    uintptr_t *end = (uintptr_t *)_end;

    for (; p < end; p++) {
        gc_mark_ptr((void *)*p);
    }
}

static inline int test_and_set_mark(uint8_t *mark) {
    uint8_t old;
#if defined(__x86_64__)
    __asm__ volatile (
        "lock xchg %0, %1"
        : "=r"(old), "+m"(*mark)
        : "0"(1)
        : "memory"
    );
#else
    old = *mark;
    *mark = 1;
#endif
    return old;
}

// Mark Phase
static void gc_mark_ptr(void *p) {
    if (!p) return;

    if (p < GLOBAL_HEAP.heap_lo || p >= GLOBAL_HEAP.heap_hi)
        return;

    if (!is_aligned(p))
        return;

    chunk_t *c = payload_to_chunk(p);

    if ((void *)c < GLOBAL_HEAP.heap_lo || (void *)c >= GLOBAL_HEAP.heap_hi)
        return;

    /* ignore pool slabs / invalid chunks */
    if (!c->inuse || c->size == 0 || c->size > GLOBAL_HEAP.total)
        return;

    if (c->magic != make_magic(c))
        return;

    /* fast mark */
    if (test_and_set_mark(&c->marked))
        return;

    /* conservative scan payload */
    uintptr_t *data = (uintptr_t *)chunk_to_payload(c);
    size_t words = c->size / sizeof(uintptr_t);

    for (size_t i = 0; i < words; i++) {
        gc_mark_ptr((void *)data[i]);
    }
}

static void gc_mark_stack(void) {
    uintptr_t sp = current_sp();
    uintptr_t *sp_ptr = (uintptr_t *) sp; 
    uintptr_t *low, *high;

    get_stack_bounds(&low, &high);

    if (sp_ptr < low) sp_ptr = low;

    for (; sp_ptr < high; sp_ptr++) {
        gc_mark_ptr((void *)*sp_ptr);
    }
}

// Sweep Phase
static void gc_sweep(void) {
    chunk_t *c = GLOBAL_HEAP.start;

    while (c) {
        if (c->inuse) {
            if (!c->marked) {
                /* unreachable */
                c->inuse = 0;
                GLOBAL_HEAP.avail += c->size;

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
    gc_mark_globals();
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
