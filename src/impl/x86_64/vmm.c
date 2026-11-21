#include "vmm.h"
#include "falloc.h"
#include "x86_64/panic.h"

static uint64_t *get_pml4(void) {
    uint64_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r"(cr3));
    return (uint64_t *)(cr3 & PAGE_ADDR_MASK);
}

static uint64_t *next_table(uint64_t *entry) {
    if (!(*entry & PAGE_PRESENT)) {
        uint64_t frame = alloc_frame();
        if (frame == 0) {
            panic("vmm: out of physical memory for page table");
        }
        uint8_t *p = (uint8_t *)frame;
        for (int i = 0; i < 4096; i++) p[i] = 0;

        *entry = frame | PAGE_PRESENT | PAGE_WRITABLE;
    }
    return (uint64_t *)(*entry & PAGE_ADDR_MASK);
}

void vmm_map_page(uint64_t virt, uint64_t phys, uint64_t flags) {
    uint64_t *pml4 = get_pml4();

    uint64_t pml4_idx = (virt >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
    uint64_t pd_idx   = (virt >> 21) & 0x1FF;
    uint64_t pt_idx   = (virt >> 12) & 0x1FF;

    uint64_t *pdpt = next_table(&pml4[pml4_idx]);
    uint64_t *pd   = next_table(&pdpt[pdpt_idx]);

    if (pd[pd_idx] & PAGE_HUGE) {
        panic("vmm: tried to map inside a 2 MiB huge page (test above 1 GiB)");
    }

    uint64_t *pt = next_table(&pd[pd_idx]);

    pt[pt_idx] = (phys & PAGE_ADDR_MASK) | flags | PAGE_PRESENT;

    asm volatile ("invlpg (%0)" : : "r"(virt) : "memory");
}

void vmm_unmap_page(uint64_t virt) {
    uint64_t *pml4 = get_pml4();

    uint64_t pml4_idx = (virt >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
    uint64_t pd_idx   = (virt >> 21) & 0x1FF;
    uint64_t pt_idx   = (virt >> 12) & 0x1FF;

    if (!(pml4[pml4_idx] & PAGE_PRESENT)) return;
    uint64_t *pdpt = (uint64_t *)(pml4[pml4_idx] & PAGE_ADDR_MASK);
    if (!(pdpt[pdpt_idx] & PAGE_PRESENT)) return;
    uint64_t *pd = (uint64_t *)(pdpt[pdpt_idx] & PAGE_ADDR_MASK);
    if (!(pd[pd_idx] & PAGE_PRESENT)) return;
    if (pd[pd_idx] & PAGE_HUGE) return;

    uint64_t *pt = (uint64_t *)(pd[pd_idx] & PAGE_ADDR_MASK);
    pt[pt_idx] = 0;

    asm volatile ("invlpg (%0)" : : "r"(virt) : "memory");
}

uint64_t vmm_get_physical(uint64_t virt) {
    uint64_t *pml4 = get_pml4();
    uint64_t pml4_idx = (virt >> 39) & 0x1FF;
    if (!(pml4[pml4_idx] & PAGE_PRESENT)) return 0;

    uint64_t *pdpt = (uint64_t *)(pml4[pml4_idx] & PAGE_ADDR_MASK);
    uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
    if (!(pdpt[pdpt_idx] & PAGE_PRESENT)) return 0;

    uint64_t *pd = (uint64_t *)(pdpt[pdpt_idx] & PAGE_ADDR_MASK);
    uint64_t pd_idx = (virt >> 21) & 0x1FF;
    if (!(pd[pd_idx] & PAGE_PRESENT)) return 0;

    if (pd[pd_idx] & PAGE_HUGE) {
        return (pd[pd_idx] & 0x000FFFFFFFE00000ULL) | (virt & 0x1FFFFF);
    }

    uint64_t *pt = (uint64_t *)(pd[pd_idx] & PAGE_ADDR_MASK);
    uint64_t pt_idx = (virt >> 12) & 0x1FF;
    if (!(pt[pt_idx] & PAGE_PRESENT)) return 0;

    return (pt[pt_idx] & PAGE_ADDR_MASK) | (virt & 0xFFF);
}