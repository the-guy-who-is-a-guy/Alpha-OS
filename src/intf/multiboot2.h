#pragma once
#include <stdint.h>

#define MULTIBOOT2_TAG_TYPE_END   0
#define MULTIBOOT2_TAG_TYPE_MMAP  6
#define MULTIBOOT2_MEMORY_AVAILABLE 1

void parse_memory_map(uint64_t mbi_addr);

struct multiboot2_info_header {
    uint32_t total_size;
    uint32_t reserved;
} __attribute__((packed));

struct multiboot2_tag {
    uint32_t type;
    uint32_t size;
} __attribute__((packed));

struct multiboot2_mmap_entry {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed));

struct multiboot2_tag_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    struct multiboot2_mmap_entry entries[]; // Flexible array member
} __attribute__((packed));