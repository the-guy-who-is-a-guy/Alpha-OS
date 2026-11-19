#include "timer.h"
#include "x86_64/port.h"

static volatile uint64_t timer_ticks = 0;

uint64_t timer_get_ticks(void) {
    return timer_ticks;
}

void timer_init(uint32_t frequency){
    uint32_t divisor = 1193180 / frequency;
    port_outb(0x43, 0x36);

    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);
    port_outb(0x40, low);
    port_outb(0x40, high);
}

void timer_handler(void) {
    timer_ticks++;
}