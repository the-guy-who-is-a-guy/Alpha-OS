#include "heap.h"
#include "vmm.h"
#include "falloc.h"
#include "x86_64/panic.h"

static uint64_t heap_start;
static uint64_t heap_end;
static uint64_t heap_next;

void heap_init(void) {
    heap_start = KERNEL_HEAP_START;
    heap_end   = KERNEL_HEAP_START + KERNEL_HEAP_SIZE;
    heap_next  = heap_start;

    uint64_t initial_size = 1024 * 1024; 
    for (uint64_t offset = 0; offset < initial_size; offset += 4096) {
        uint64_t phys = alloc_frame();
        if (phys == 0) {
            panic("heap_init: out of physical memory");
        }
        vmm_map_page(heap_start + offset, phys, PAGE_WRITABLE);
    }
}

void* kmalloc(size_t size) {
    if (size == 0) return NULL;

    size = (size + 7) & ~7ULL;

    if (heap_next + size > heap_end) {
        return NULL;
    }

    void* ptr = (void*)heap_next;
    heap_next += size;
    return ptr;
}

void kfree(void* ptr) {
    (void)ptr;
}