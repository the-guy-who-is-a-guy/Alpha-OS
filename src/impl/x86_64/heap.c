#include "heap.h"
#include "vmm.h"
#include "falloc.h"
#include "x86_64/panic.h"
#include "string.h"
#include "print.h"

#define HEAP_MAGIC 0xB10C

struct block {
    size_t        size;
    struct block *next;
    uint32_t      magic;
    uint32_t      free;
    uint64_t      reserved;
};

static struct block *free_list = NULL;
static uint64_t      heap_start = 0;
static uint64_t      heap_end   = 0;
static uint64_t      heap_next  = 0;

static size_t align_up(size_t n) {
    if (n > (size_t)-16) return 0;
    return (n + 15) & ~(size_t)15;
}

#define BLOCK_HEADER_SIZE sizeof(struct block)

static struct block *grow_heap(size_t bytes) {
    if (bytes == 0) return NULL;
    bytes = align_up(bytes);
    if (bytes == 0) return NULL;
    if (heap_next > heap_end || bytes > heap_end - heap_next) {
        return NULL;
    }

    uint64_t start = heap_next;
    uint64_t end   = start + bytes;

    kprintf("grow_heap: start=0x%x end=0x%x\n", start, end);

    for (uint64_t addr = start; addr < end; addr += 4096) {
        uint64_t phys = alloc_frame();
        if (phys == 0) {
            return NULL;
        }
        vmm_map_page(addr, phys, PAGE_WRITABLE);
    }

    struct block *b = (struct block *)start;
    b->size  = bytes;
    b->next  = NULL;
    b->magic = HEAP_MAGIC;
    b->free  = 1;
    b->reserved = 0;

    heap_next = end;
    return b;
}


void heap_init(void) {
    heap_start = KERNEL_HEAP_START;
    heap_end   = KERNEL_HEAP_START + KERNEL_HEAP_SIZE;
    heap_next  = heap_start;

    size_t initial = KERNEL_HEAP_SIZE;
    struct block *b = grow_heap(initial);
    if (!b) {
        panic("heap_init: out of physical memory");
    }

    free_list = b;
}

void *kmalloc(size_t size) {
    if (size == 0) return NULL;

    size = align_up(size);
    if (size == 0 || size > (size_t)-BLOCK_HEADER_SIZE) return NULL;
    size_t needed = size + BLOCK_HEADER_SIZE;

    /* Walk the free list for a block large enough */
    struct block *prev = NULL;
    struct block *b    = free_list;

    while (b) {
        if (b->magic != HEAP_MAGIC) {
            panic("kmalloc: corrupted free list (magic)");
        }
        if (!b->free) {
            panic("kmalloc: allocated block in free list");
        }

        if (b->size >= needed) {
            /* Found a block. Split it if it's much larger than needed. */
            size_t remaining = b->size - needed;

            if (remaining >= sizeof(struct block) + 16) {
                /* Split: b keeps `needed`, the tail becomes a new free block */
                struct block *tail = (struct block *)((uint8_t *)b + needed);
                kprintf("split: b=0x%x size=0x%x needed=0x%x tail=0x%x\n", b, b->size, needed, tail);
                tail->size  = remaining;
                tail->next  = b->next;
                tail->magic = HEAP_MAGIC;
                tail->free  = 1;
                tail->reserved = 0;

                b->size = needed;
                b->next = tail;
            }

            /* Remove b from the free list */
            if (prev) prev->next = b->next;
            else      free_list  = b->next;

            b->free  = 0;
            b->magic = HEAP_MAGIC;

            /* Return the memory after the header */
            return (void *)((uint8_t *)b + sizeof(struct block));
        }

        prev = b;
        b    = b->next;
    }

    /* No free block large enough. Grow the heap. */
    struct block *nb = grow_heap(needed > 4096 ? needed : 4096);
    if (!nb) {
        return NULL;   /* out of memory */
    }

    /* Insert the new block at the front of the free list */
    nb->next = free_list;
    free_list = nb;

    /* Retry (the new block is large enough) */
    return kmalloc(size);
}

void kfree(void *ptr) {
    if (!ptr) return;

    uint64_t address = (uint64_t)ptr;
    if (address < heap_start + BLOCK_HEADER_SIZE ||
        address > heap_end ||
        (address & 15) != 0) {
        panic("kfree: invalid heap pointer");
    }

    struct block *b = (struct block *)(address - BLOCK_HEADER_SIZE);
    if (((uint64_t)b & 15) != 0 ||
        b->magic != HEAP_MAGIC ||
        b->free > 1 ||
        b->size < BLOCK_HEADER_SIZE ||
        (b->size & 15) != 0 ||
        b->size > heap_end - (uint64_t)b) {
        panic("kfree: invalid pointer (magic or size)");
    }
    if (b->free) return;

    b->free = 1;

    /* Insert into the free list, sorted by address, and coalesce */
    struct block *prev = NULL;
    struct block *cur  = free_list;

    while (cur && (uint64_t)cur < (uint64_t)b) {
        prev = cur;
        cur  = cur->next;
    }

    b->next = cur;
    if (prev) prev->next = b;
    else      free_list  = b;

    /* Coalesce with next block */
    if (cur && (uint64_t)b + b->size == (uint64_t)cur) {
        b->size += cur->size;
        b->next  = cur->next;
    }

    /* Coalesce with previous block */
    if (prev && (uint64_t)prev + prev->size == (uint64_t)b) {
        prev->size += b->size;
        prev->next  = b->next;
    }
}
