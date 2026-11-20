#pragma once
#include <stdint.h>

#define MAX_MEMORY_RANGES 32

struct MemoryRange {
    uint64_t base;
    uint64_t length;
};

extern struct MemoryRange usable_ranges[MAX_MEMORY_RANGES];
extern int usable_range_count;

void frames_init(void);

uint64_t alloc_frame(void);
void free_frame(uint64_t addr);
uint64_t frames_free_count(void);