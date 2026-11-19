#include "shell.h"
#include "keyboard.h"
#include "print.h"
#include "string.h"
#include "timer.h"
#include "x86_64/rtc.h"

void readline(char *buf, size_t max) {
    size_t i = 0;
    while (1) {
        char c = keyboard_getchar();
        if (c == 0) continue;

        if (c == '\n') {
            buf[i] = '\0';
            print_char('\n');
            return;
        }
        if (c == '\b') {
            if (i > 0) {
                i--;
                print_string("\b \b");
            }
            continue;
        }
        if (i < max - 1) {
            buf[i++] = c;
            print_char(c);
        }
    }
}

static void cmd_help(void) {
    print_string("commands:\n");
    print_string("  help     - this message\n");
    print_string("  clear    - clear screen\n");
    print_string("  uptime   - seconds since boot\n");
    print_string("  rtc      - current RTC seconds\n");
    print_string("  echo X   - print X\n");
    print_string("  reboot   - halt (no ACPI reset yet)\n");
}

static void cmd_echo(const char *arg) {
    print_string(arg);
    print_char('\n');
}

void handle_command(const char *cmd) {
    if (cmd[0] == '\0') return;

    if (strcmp(cmd, "help") == 0) {
        cmd_help();
    } else if (strcmp(cmd, "clear") == 0) {
        print_clear();
    } else if (strcmp(cmd, "uptime") == 0) {
        print_string("uptime: ");
        print_uint64_dec(timer_get_ticks() / 100);
        print_string("s\n");
    } else if (strcmp(cmd, "rtc") == 0) {
        print_string("rtc seconds: ");
        print_uint64_dec(rtc_seconds());
        print_char('\n');
    } else if (strncmp(cmd, "echo ", 5) == 0) {
        cmd_echo(cmd + 5);
    } else if (strcmp(cmd, "reboot") == 0) {
        print_string("halting (no reset yet)\n");
        asm volatile ("cli");
        while (1) asm volatile ("hlt");
    } else {
        print_string("unknown: ");
        print_string(cmd);
        print_char('\n');
    }
}
