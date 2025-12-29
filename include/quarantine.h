#pragma once
#include "allocator.h"

#define QUARANTINE_MAX 32

void quarantine_init(void);
void quarantine_push(chunk_t * c);
void quarantine_flush_one(void);
void quarantine_flush_all();

int get_quarantine_size();