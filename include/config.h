#pragma once

#define HEAP_DEFAULT_SIZE (8 * 1024 * 1024)
#define HEAP_ALIGNMENT 16

#define CHUNK_MAGIC 0xDEADC0DE

#define ENABLE_POISONING 0
#define ENABLE_DEBUG 1
#define ENABLE_QUARANTINE 1
#define ENABLE_SPRAY_DETECT 1
#define ENABLE_GC 1

#define POOL_MAX 256

static const size_t pool_sizes[] = {
    16, 32, 64, 128, 256
};

#define POOL_COUNT 5

#define SIZE_T_ONE ((size_t) 1)
#define CHUNK_ALIGN_MASK (HEAP_ALIGNMENT - SIZE_T_ONE)

#define is_aligned(A) (((size_t)((A)) & (CHUNK_ALIGN_MASK)) == 0)

#define align_offset(A) ((((size_t)(A) & CHUNK_ALIGN_MASK) == 0) ? 0 : ((HEAP_ALIGNMENT - ((size_t)(A) & CHUNK_ALIGN_MASK)) & CHUNK_ALIGN_MASK))