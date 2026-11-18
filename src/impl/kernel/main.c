#include "print.h"
#include "x86_64/rtc.h"
#include "keyboard.h"
#include "keycodes.h"

// Input handler
void handle_input(struct KeyboardEvent event) {
    if (event.type == KEYBOARD_EVENT_TYPE_MAKE) {
        
    }
    if (event.type == KEYBOARD_EVENT_TYPE_BREAK) {

    }
}

void kernel_main(){
    print_clear();
    print_set_color(COLOR_YELLOW, COLOR_BLACK);
    print_string("ALPHA OS - BOOTED\n");

    keyboard_init();
    print_set_color(COLOR_GREEN, COLOR_BLACK);
    print_string("INITIALIZED KEYBOARD\n");
    
    keyboard_set_handler(handle_input);
    print_string("INITIALIZED KEYBOARD HANDLER\n");
/*
    uint8_t prev_seconds = 0;

    for (uint8_t i = 0; i < 5;){
        uint8_t seconds = rtc_seconds();

        if (seconds != prev_seconds) {
            i++;
            print_set_color(COLOR_GREEN, COLOR_BLACK);
            print_string("Seconds: ");
            print_uint64_dec(seconds);
            print_string("\n");
            prev_seconds = seconds;
        }
    }
    print_string(" - Seconds loop disabled. \n");
*/
    while (1){__asm__ volatile ("hlt");};
}