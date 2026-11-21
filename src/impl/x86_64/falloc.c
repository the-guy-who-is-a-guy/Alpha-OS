#include "falloc.h"
#include "x86_64/serial.h"
#include "x86_64/panic.h"

/* 128 MiB worth of frames = 32768 frames = 4096 bytes of bitmap. */
#define MAX_FRAMES      (128 * 1024 * 1024 / 4096)
#define FRAME_SIZE      4096
#define BITMAP_BYTES    (MAX_FRAMES / 8)
#define KERNEL_PHYS_START 0x100000ULL
#define KERNEL_VMA      0xFFFF800000000000ULL

static uint8_t frame_bitmap[BITMAP_BYTES];

/* Filled by parse_memory_map() before frames_init() runs. */
struct MemoryRange usable_ranges[MAX_MEMORY_RANGES];
int usable_range_count = 0;

/* The boot image starts at 1 MiB; _kernel_end is a higher-half virtual address. */
extern uint8_t _kernel_start;
extern uint8_t _kernel_end;

/* ------------------------------------------------------------------ */
/*  Helpers                                                            */
/* ------------------------------------------------------------------ */

static inline void bitmap_set(uint64_t frame) {
    frame_bitmap[frame / 8] |= (uint8_t)(1 << (frame % 8));
}

static inline void bitmap_clear(uint64_t frame) {
    frame_bitmap[frame / 8] &= (uint8_t)~(1 << (frame % 8));
}

static inline int bitmap_test(uint64_t frame) {
    return frame_bitmap[frame / 8] & (1 << (frame % 8));
}

/* ------------------------------------------------------------------ */
/*  Init                                                               */
/* ------------------------------------------------------------------ */

void frames_init(void) {
    serial_write_string("frames_init: usable_range_count = ");
    serial_write_hex64(usable_range_count);
    serial_write_string("\n");
    if (usable_range_count == 0) {
        panic("frames_init: no usable ranges (parse_memory_map not called?)");
    }

    /* 1. Mark everything used. */
    for (uint64_t i = 0; i < BITMAP_BYTES; i++) {
        frame_bitmap[i] = 0xFF;
    }

    /* 2. Free the usable ranges reported by Multiboot. */
    for (int r = 0; r < usable_range_count; r++) {
        uint64_t base   = usable_ranges[r].base;
        uint64_t length = usable_ranges[r].length;

        uint64_t start_frame = (base + FRAME_SIZE - 1) / FRAME_SIZE;  /* round up */
        uint64_t end_frame   = (base + length) / FRAME_SIZE;          /* round down */

        for (uint64_t f = start_frame; f < end_frame && f < MAX_FRAMES; f++) {
            bitmap_clear(f);
        }
    }

    /* 3. Reserve everything below 1 MiB (BIOS, IVT, BDA, Multiboot info). */
    for (uint64_t f = 0; f < 0x100000 / FRAME_SIZE; f++) {
        bitmap_set(f);
    }

    /* 4. Reserve the boot image and higher-half kernel. */
    uint64_t k_start = KERNEL_PHYS_START / FRAME_SIZE;
    uint64_t k_end   = (((uint64_t)&_kernel_end - KERNEL_VMA) + FRAME_SIZE - 1) / FRAME_SIZE;

    for (uint64_t f = k_start; f < k_end && f < MAX_FRAMES; f++) {
        bitmap_set(f);
    }
}

/* ------------------------------------------------------------------ */
/*  Alloc / free                                                       */
/* ------------------------------------------------------------------ */

uint64_t alloc_frame(void) {
    for (uint64_t byte = 0; byte < BITMAP_BYTES; byte++) {
        if (frame_bitmap[byte] == 0xFF) continue;   /* all 8 used, skip */
        for (int bit = 0; bit < 8; bit++) {
            if (!(frame_bitmap[byte] & (1 << bit))) {
                frame_bitmap[byte] |= (uint8_t)(1 << bit);
                uint64_t frame = byte * 8 + bit;
                return frame * FRAME_SIZE;
            }
        }
    }
    return 0;   /* OOM — frame 0 is reserved, so 0 is an unambiguous sentinel */
}

void free_frame(uint64_t addr) {
    uint64_t frame = addr / FRAME_SIZE;
    if (frame == 0 || frame >= MAX_FRAMES) {
        return;   /* ignore double-free of sentinel and out-of-range */
    }
    bitmap_clear(frame);
}

/* ------------------------------------------------------------------ */
/*  Stats                                                              */
/* ------------------------------------------------------------------ */

uint64_t frames_free_count(void) {
    uint64_t free = 0;
    for (uint64_t byte = 0; byte < BITMAP_BYTES; byte++) {
        for (int bit = 0; bit < 8; bit++) {
            if (!(frame_bitmap[byte] & (1 << bit))) {
                free++;
            }
        }
    }
    return free;
}