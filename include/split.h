#pragma once
#include "allocator.h"

chunk_t *chunk_split(chunk_t *chunk, size_t payload);