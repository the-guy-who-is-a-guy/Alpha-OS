#include "multiboot2.h"
#include "falloc.h"
#include "x86_64/serial.h"

void parse_memory_map(uint64_t mbi_addr) {
    struct multiboot2_info_header *header = (void *)mbi_addr;
    uint8_t *ptr = (uint8_t *)mbi_addr + sizeof(*header);
    uint8_t *end = (uint8_t *)mbi_addr + header->total_size;

    while (ptr < end) {
        struct multiboot2_tag *tag = (void *)ptr;
        if (tag->type == MULTIBOOT2_TAG_TYPE_END) break;

        if (tag->type == MULTIBOOT2_TAG_TYPE_MMAP) {
            struct multiboot2_tag_mmap *mmap = (void *)tag;
            uint8_t *entry_ptr = (uint8_t *)mmap->entries;
            uint8_t *tag_end = (uint8_t *)tag + tag->size;

            while (entry_ptr < tag_end) {
                struct multiboot2_mmap_entry *entry = (void *)entry_ptr;
                if (entry->type == MULTIBOOT2_MEMORY_AVAILABLE) {
                    if (usable_range_count < MAX_MEMORY_RANGES) {
                        usable_ranges[usable_range_count].base   = entry->base_addr;
                        usable_ranges[usable_range_count].length = entry->length;
                        usable_range_count++;
                        serial_write_string("Available RAM: 0x");
                        serial_write_hex64(entry->base_addr);
                        serial_write_string(" - 0x");
                        serial_write_hex64(entry->base_addr + entry->length);
                        serial_write_string("\n");
                    }
                }
                entry_ptr += mmap->entry_size;
            }
        }

        ptr += tag->size;
        ptr = (uint8_t *)(((uintptr_t)ptr + 7) & ~7);
    }
}