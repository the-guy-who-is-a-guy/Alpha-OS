#pragma once
#include <stdint.h>

#define PAGE_PRESENT   (1ULL << 0)
#define PAGE_WRITABLE  (1ULL << 1)
#define PAGE_USER      (1ULL << 2)
#define PAGE_PWT       (1ULL << 3)
#define PAGE_PCD       (1ULL << 4)
#define PAGE_ACCESSED  (1ULL << 5)
#define PAGE_DIRTY     (1ULL << 6)
#define PAGE_HUGE      (1ULL << 7)
#define PAGE_GLOBAL    (1ULL << 8)
#define PAGE_NX        (1ULL << 63)

#define PAGE_ADDR_MASK 0x000FFFFFFFFFF000ULL

void     vmm_map_page(uint64_t virt, uint64_t phys, uint64_t flags);
void     vmm_unmap_page(uint64_t virt);
uint64_t vmm_get_physical(uint64_t virt);