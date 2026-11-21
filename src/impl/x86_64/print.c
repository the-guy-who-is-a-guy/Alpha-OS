#include "print.h"
#include "heap.h"
#include <stdarg.h>
#include "string.h"
#include "x86_64/serial.h"

const static size_t NUM_COLS = 80;
const static size_t NUM_ROWS = 25;

struct Char {
    uint8_t character;
    uint8_t color;
};

struct Char* buffer = (struct Char*) 0xb8000;
size_t col = 0;
size_t row = 0;
uint8_t color = COLOR_WHITE | COLOR_BLACK << 4;

void clear_row(size_t row) {
    struct Char empty = (struct Char) { character: ' ', color: color};
    for (size_t col = 0; col < NUM_COLS; col++) {
        buffer[col + NUM_COLS * row] = empty;
    }
}

void print_clear() {
    for (size_t i = 0; i < NUM_ROWS; i++) {
        clear_row(i);
    }
    row = 0;
    col = 0;
}

void newline(){
    col = 0;
    if (row < NUM_ROWS - 1) {
        row++;
        return;
    }

    for (size_t r = 1; r < NUM_ROWS; r++) {
        for (size_t c = 0; c < NUM_COLS; c++) {
            struct Char character = buffer[c + NUM_COLS * r];
            buffer[c + NUM_COLS * (r - 1)] = character;
        }
    }

    clear_row(NUM_ROWS - 1);
}

void print_char(char c) {
    if (c == '\n') { newline(); return; }

    if (c == '\b') {
        if (col > 0) {
            col--;
        } else if (row > 0) {
            row--;
            col = NUM_COLS - 1;
        }
        return;
    }

    if (c == '\r') {
        col = 0; return;
    }

    if (col >= NUM_COLS) newline();

    buffer[col + NUM_COLS * row] = (struct Char){ .character = (uint8_t)c, .color = color };
    col++;
}

void print_string(char* str) {
    for (size_t i = 0; 1; i++) {
        char c = (uint8_t) str[i];

        if (c == '\0') {
            return;
        }
        print_char(c);
    }
}

void print_set_color(uint8_t foreground, uint8_t background) {
    color = foreground + (background << 4);
}

void print_uint64_dec(uint64_t value) {
    if (value == 0) {
        print_char('0');
        return;
    }

    char buffer[20];
    int i = 0;

    while (value > 0) {
        buffer[i++] = (value % 10) + '0';
        value /= 10;
    }

    while (i-- > 0) {
        print_char(buffer[i]);
    }
    return;
}

void print_uint64_hex(uint64_t value) {
    if (value == 0) {
        print_char('0');
        return;
    }

    char buffer[16];
    int i = 0;

    while (value > 0) {
        uint8_t digit = value & 0xF;
        if (digit < 10){
            buffer[i++] = digit + '0';
        } else {
            buffer[i++] = digit - 10 + 'A';
        }
        value >>= 4;
    }

    while (i-- > 0) {
        print_char(buffer[i]);
    }
    return;
}

void print_uint64_bin(uint64_t value) {
    char buffer[64];

    for (size_t i = 0; i < 64; i++) {
        buffer[i] = (value & 1) + '0';
        value >>= 1;
    }

    for (size_t i = 64; i > 0; i--) {
        print_char(buffer[i - 1]);
    }
    return;
}
void kprintf(const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof buf, fmt, args);
    va_end(args);

    print_string(buf);
    serial_write_string(buf);
}