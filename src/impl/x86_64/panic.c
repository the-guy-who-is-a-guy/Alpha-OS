#include "x86_64/serial.h"
#include "print.h"

void panic(const char *message) {
    // 1. Disable interrupts immediately so nothing else runs
    asm volatile ("cli");

    // 2. Print to serial (survives screen corruption)
    serial_write_string("\n*** KERNEL PANIC ***\n");
    serial_write_string(message);
    serial_write_string("\n");

    // 3. Also print to VGA (in case you are looking at the screen)
    print_set_color(COLOR_WHITE, COLOR_RED);
    print_string("\n*** KERNEL PANIC ***\n");
    print_string(message);
    print_string("\n");

    // 4. Print CR2 (faulting address) if this is a page fault
    // (You can read it with inline asm: `mov %%cr2, %0`)

    // 5. Halt the CPU forever
    while (1) {
        asm volatile ("hlt");
    }
}