#include "x86_64/rtc.h"
#include "x86_64/port.h"

#define PORT_COMMAND 0x70
#define    PORT_DATA 0x71

#define REGISTER_SECONDS 0x00
#define  REGISTER_STAT_A 0x0A
#define  REGISTER_STAT_B 0x0B

#define RTC_UPDATE_IN_PROGRESS (1<<7)
#define DATA_MODE (1<<2)

uint8_t rtc_read_register(uint8_t reg) {
    port_outb(PORT_COMMAND, reg);
    return port_inb(PORT_DATA);
}

void rtc_wait() {
    while ((rtc_read_register(REGISTER_STAT_A) & RTC_UPDATE_IN_PROGRESS));
}

uint8_t rtc_is_bcd() {
    return !(rtc_read_register(REGISTER_STAT_B) & DATA_MODE);
}

uint8_t rtc_seconds() {
    uint8_t seconds_a;
    uint8_t seconds_b;
    uint8_t is_bcd = rtc_is_bcd();

    do {
        rtc_wait();
        seconds_a = rtc_read_register(REGISTER_SECONDS);
        rtc_wait();
        seconds_b = rtc_read_register(REGISTER_SECONDS);
    } while (seconds_a != seconds_b);

    if (is_bcd) {
        return (seconds_a & 0x0F) + ((seconds_a & 0xF0) >> 4) * 10;
    }

    return seconds_a;
}