#pragma once
#include <stddef.h>
#include <stdint.h>

#define KERNEL_HEAP_START 0xFFFF800000400000ULL
#define KERNEL_HEAP_SIZE  (16 * 1024 * 1024)

void  heap_init(void);
void* kmalloc(size_t size);
void  kfree(void* ptr);