#pragma once

#include <stdint.h>
#include <stdbool.h>

#define KEY_LEFT   0x11   /* Ctrl+Q — unused in shells */
#define KEY_RIGHT  0x12
#define KEY_UP     0x13
#define KEY_DOWN   0x14

enum {
    KEYBOARD_EVENT_TYPE_MAKE = 0,
    KEYBOARD_EVENT_TYPE_BREAK = 1,
};

struct KeyboardEvent 
{
    uint8_t type;
    uint16_t code;
};

struct KeyboardBuffer{
    struct KeyboardEvent events[256];
    volatile uint32_t head;
    volatile uint32_t tail;
};

bool keyboard_buffer_push(struct KeyboardBuffer *buffer, struct KeyboardEvent event);
bool keyboard_buffer_pop(struct KeyboardBuffer *buffer, struct KeyboardEvent *out);

bool keyboard_buffer_empty(const struct KeyboardBuffer *buffer);
bool keyboard_buffer_full(const struct KeyboardBuffer *buffer);

void keyboard_buffer_clear(struct KeyboardBuffer *buffer);

void keyboard_init(void);
void keyboard_set_handler(void (*handler)(struct KeyboardEvent event));

void keyboard_process(void);

bool keyboard_has_event(void);

char keyboard_getchar(void);

bool keyboard_shift_pressed(void);

bool keyboard_caps_lock(void);