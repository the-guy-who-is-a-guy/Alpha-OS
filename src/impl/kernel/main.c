#include "print.h"
#include "x86_64/rtc.h"
#include "keyboard.h"
#include "keycodes.h"
#include "timer.h"
#include "x86_64/port.h"
#include "idt.h"
#include "shell.h"

#define DEBUG_TEST 0
#define TIMER_HZ 100

// Input handler
void handle_input(struct KeyboardEvent event) {
    if (DEBUG_TEST == 1) {
        if (event.type == KEYBOARD_EVENT_TYPE_MAKE) {
            print_set_color(COLOR_GREEN, COLOR_BLACK);
            print_string("PRESSED");
            print_char(to_ascii(event.code, keyboard_shift_pressed(), keyboard_caps_lock()));
            print_string("\n");
        }
        if (event.type == KEYBOARD_EVENT_TYPE_BREAK) {
            print_set_color(COLOR_RED, COLOR_BLACK);
            print_string("RELEASED");
            print_char(to_ascii(event.code, keyboard_shift_pressed(), keyboard_caps_lock()));
            print_string("\n");
        }
    }
}

void sleep_ms(uint64_t ms) {
    uint64_t target = timer_get_ticks() + (ms / 10);
    while (timer_get_ticks() < target) {
        asm volatile ("sti\n\thlt" ::: "memory");
    }
}

uint64_t last_second = 0;

void kernel_main(){
    print_clear();
    print_set_color(COLOR_YELLOW, COLOR_BLACK);
    print_string("ALPHA OS - BOOTED\n");

    idt_init();
    timer_init(100);
    idt_set_handler_timer(timer_handler);

    keyboard_init();
    print_set_color(COLOR_GREEN, COLOR_BLACK);
    print_string("INITIALIZED KEYBOARD\n");
    
    keyboard_set_handler(handle_input);
    print_string("INITIALIZED KEYBOARD HANDLER\n");
    asm volatile ("sti");
    
    print_string("Mask master: 0x");
    print_uint64_hex(port_inb(0x21));
    print_char('\n');
    print_string("Ticks at start: ");
    print_uint64_dec(timer_get_ticks());
    print_char('\n');
    print_string("PIC mask: 0x");
    print_uint64_hex(port_inb(0x21));   // master IMR
    print_char('\n');

    uint64_t last_second = 0;
    
    char line[128];
    print_string("> ");
    while (1){
        readline(line, sizeof line);
        handle_command(line);
        print_string("> ");
    }
}