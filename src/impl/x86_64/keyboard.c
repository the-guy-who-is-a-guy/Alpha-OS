#include "keyboard.h"

#include <stddef.h>
#include "idt.h"
#include "x86_64/ps2.h"
#include "keycodes.h"
#include "print.h"

void (*keyboard_handler_user)(struct KeyboardEvent event);

#define KEYBOARD_EXTENDED_SCAN_CODE 0xE0

static bool shift_pressed = false;
static bool caps_lock_on = false;

static inline uint32_t buffer_next(uint32_t i){
    return (i + 1) & (255);
}

bool keyboard_buffer_push(struct KeyboardBuffer *buffer, struct KeyboardEvent event){
    uint32_t next = buffer_next(buffer->head);
    if (next == buffer->tail) {
        return false;
    }
    buffer->events[buffer->head] = event;
    buffer->head = next;
    return true;
}

bool keyboard_buffer_pop(struct KeyboardBuffer *buffer, struct KeyboardEvent *output){
    if (buffer->head == buffer->tail) {
        return false;
    }
    *output = buffer->events[buffer->tail];
    buffer->tail = buffer_next(buffer->tail);
    return true;
}

bool keyboard_buffer_full(const struct KeyboardBuffer *buffer){
    return buffer_next(buffer->head) == buffer->tail;
}

bool keyboard_buffer_empty(const struct KeyboardBuffer *buffer) {
    return buffer->head == buffer->tail;
}

void keyboard_buffer_clear(struct KeyboardBuffer *buffer){
    buffer->head = 0;
    buffer->tail = 0;
}

static struct KeyboardBuffer kb_buffer;

bool keyboard_has_event(void) {
    return !keyboard_buffer_empty(&kb_buffer);
}

void keyboard_handler() {
    static bool is_extended = 0;

    uint8_t scan_code = ps2_read_scan_code();

    if (scan_code == KEYBOARD_EXTENDED_SCAN_CODE) {
        is_extended = true;
        return;
    }

    if (keyboard_handler_user == NULL) {
        return;
    }

    uint16_t fat_code = scan_code & 0x7F;
    bool is_break = (scan_code & 0x80) != 0;

    if (is_extended) {
        is_extended = false;
        fat_code |= KEYBOARD_EXTENDED_SCAN_CODE << 8;
    }

    if (fat_code == KEY_CODE_LEFT_SHIFT | fat_code == KEY_CODE_RIGHT_SHIFT){
        shift_pressed = !is_break;
        return;
    }
    if (fat_code == KEY_CODE_CAPS_LOCK){
        if (!is_break){
            caps_lock_on = !caps_lock_on;
        }
        return;
    }

    struct KeyboardEvent event;

    event.type = is_break ? KEYBOARD_EVENT_TYPE_BREAK
                          : KEYBOARD_EVENT_TYPE_MAKE;
    event.code = fat_code;

    keyboard_buffer_push(&kb_buffer, event);
}

void keyboard_init(void){
    keyboard_buffer_clear(&kb_buffer);
    idt_set_handler_keyboard(keyboard_handler);
}

void keyboard_set_handler(void (*handler)(struct KeyboardEvent event)){
    keyboard_handler_user = handler;
}

void keyboard_process(void) {
    struct KeyboardEvent event;
    while (keyboard_buffer_pop(&kb_buffer, &event)) {
        if (keyboard_handler_user != NULL) {
            keyboard_handler_user(event);
        }
    }
}

char keyboard_getchar(void) {
    struct KeyboardEvent ev;
    while (!keyboard_buffer_pop(&kb_buffer, &ev)) {
        asm volatile ("sti\n\thlt" ::: "memory");
    }
    if (ev.type == KEYBOARD_EVENT_TYPE_BREAK) {
        return 0;
    }

    char c = to_ascii(ev.code, shift_pressed, caps_lock_on);
    if (c == 0 && ev.code != 0x2A && ev.code != 0x36) {  /* not shift */
        print_set_color(COLOR_RED, COLOR_BLACK);
        print_string("[UNMAPPED:");
        print_uint64_hex(ev.code);
        print_string("]");
    }
    return c;

    return to_ascii(ev.code, shift_pressed, caps_lock_on);
}

bool keyboard_shift_pressed(void) { return shift_pressed; }
bool keyboard_caps_lock(void) { return caps_lock_on; }