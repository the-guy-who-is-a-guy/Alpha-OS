#include "x86_64/port.h"

#define PORT 0x3f8

int serial_init(void) {
    port_outb(PORT + 1, 0x00);
    port_outb(PORT + 3, 0x80);
    port_outb(PORT + 0, 0x03);
    port_outb(PORT + 1, 0x00);
    port_outb(PORT + 3, 0x03);
    port_outb(PORT + 2, 0xC7);
    port_outb(PORT + 4, 0x0B);
    return 0;
}

static int is_transmit_empty(void) {
    return port_inb(PORT + 5) & 0x20;
}

void serial_write(char a) {
    while (is_transmit_empty() == 0);
    port_outb(PORT, a);
}

void serial_write_string(const char *str) {
    while (*str) {
        serial_write(*str++);
    }
}

void serial_write_hex64(uint64_t value) {
    static const char digits[] = "0123456789abcdef";
    int started = 0;

    for (int shift = 60; shift >= 0; shift -= 4) {
        uint8_t nib = (value >> shift) & 0xF;
        if (nib != 0 || started || shift == 0) {
            started = 1;
            serial_write(digits[nib]);
        }
    }
}