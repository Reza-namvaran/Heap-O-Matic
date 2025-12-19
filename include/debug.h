#pragma once
#include "allocator.h"

#ifdef ENABLE_DEBUG

int heap_check(void);
void heap_dump(void);

#else

static inline int heap_check(void) { return 0; }
static inline void heap_dump(void) {}

#endif