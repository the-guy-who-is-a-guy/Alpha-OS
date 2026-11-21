#include "x86_64/serial.h"
#include "print.h"

void panic(const char *message) {
    // 1. Disable interrupts immediately so nothing else runs
    asm volatile ("cli");

    // 2. Print to serial (survives screen corruption)
    print_set_color(COLOR_WHITE, COLOR_RED);
    kprintf("\n*** KERNEL PANIC ***\n%c\n", message);

    // 4. Halt the CPU forever
    while (1) {
        asm volatile ("hlt");
    }
}