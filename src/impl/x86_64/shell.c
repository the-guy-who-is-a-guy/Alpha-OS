#include "shell.h"
#include "keyboard.h"
#include "print.h"
#include "string.h"
#include "timer.h"
#include "x86_64/rtc.h"
#include "falloc.h"
#include "vmm.h"
#include "heap.h"

static size_t prev_len = 0;
static void redraw_line(const char *buf, size_t len, size_t pos, const char* prompt) {
    print_char('\r');
    print_string(prompt);
    for (size_t i = 0; i < len; i++) print_char(buf[i]);
    for (size_t i = len; i < prev_len; i++) print_char(' ');
    for (size_t i = (len > prev_len ? len : prev_len); i > pos; i--) print_char('\b');
    prev_len = len;
}

void readline(char *buf, size_t max, const char *prompt) {
    size_t len = 0;
    size_t pos = 0;
    static size_t prev_len = 0;

    while (1) {
        char c = keyboard_getchar();
        if (c == 0) continue;

        if (c == KEY_LEFT)  {
            if (pos > 0) {
                pos--;
                print_char('\b');
            }
            continue;
        }
        if (c == KEY_RIGHT) {
            if (pos < len) {
                pos++;
                print_char(buf[pos - 1]);
            }
            continue;
        }
        if (c == KEY_UP)    { /* history up        */ continue; }
        if (c == KEY_DOWN)  { /* history down      */ continue; }

        if (c == '\n') {
            buf[len] = '\0';
            print_char('\n');
            prev_len = 0;
            return;
        }

        if (c == '\b') {
            if (pos > 0) {
                for (size_t i = pos; i < len; i++) buf[i - 1] = buf[i];
                pos--;
                len--;
                buf[len] = '\0';
                redraw_line(buf, len, pos, prompt);
            }
            continue;
        }

        if (len < max - 1) {
            for (size_t i = len; i > pos; i--) buf[i] = buf[i - 1];
            buf[pos] = c;
            pos++;
            len++;
            buf[len] = '\0';
            redraw_line(buf, len, pos, prompt);
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

static void cmd_echo(char *arg) {
    print_string(arg);
    print_char('\n');
}

static void cmd_maptest(void) {
    const uint64_t virt = 0x40000000ULL;

    uint64_t frame = alloc_frame();
    if (frame == 0) {
        print_string("maptest: alloc_frame failed\n");
        return;
    }
    print_string("maptest: got frame 0x");
    print_uint64_hex(frame);
    print_char('\n');

    vmm_map_page(virt, frame, PAGE_WRITABLE);
    print_string("maptest: mapped 0x40000000 -> frame\n");

    volatile uint64_t *p = (volatile uint64_t *)virt;
    *p = 0xDEADBEEFCAFEBABEULL;
    print_string("maptest: wrote 0xdeadbeefcafebabe\n");

    if (*p == 0xDEADBEEFCAFEBABEULL) {
        print_string("maptest: read back OK\n");
    } else {
        print_string("maptest: read back MISMATCH: 0x");
        print_uint64_hex(*p);
        print_char('\n');
    }

    uint64_t phys = vmm_get_physical(virt);
    print_string("maptest: vmm_get_physical -> 0x");
    print_uint64_hex(phys);
    print_char('\n');

    vmm_unmap_page(virt);
    print_string("maptest: unmapped\n");
}

static void cmd_splittest(void) {
    const uint64_t virt = 0x200000;   /* 2 MiB — inside the identity map */

    /* Before: read the current value at 0x200000 (some kernel data). */
    uint64_t before = *(volatile uint64_t *)virt;
    print_string("splittest: before = 0x");
    print_uint64_hex(before);
    print_char('\n');

    /* Map a new frame at 0x200000. This forces a split. */
    uint64_t frame = alloc_frame();
    if (frame == 0) { print_string("alloc failed\n"); return; }

    vmm_map_page(virt, frame, PAGE_WRITABLE);

    /* Write a marker to the new mapping. */
    *(volatile uint64_t *)virt = 0xDEADBEEFCAFEBABEULL;

    /* Read it back — should be the marker, not the old value. */
    uint64_t after = *(volatile uint64_t *)virt;
    print_string("splittest: after  = 0x");
    print_uint64_hex(after);
    print_char('\n');

    /* Also check a neighbor address — it should still show the old data
       (the split preserved the identity mapping for the other 511 pages). */
    uint64_t neighbor = *(volatile uint64_t *)(virt + 0x1000);
    print_string("splittest: neighbor = 0x");
    print_uint64_hex(neighbor);
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
    } else if (strcmp(cmd, "alloc") == 0) {
        uint64_t f = alloc_frame();
        if (f == 0) {
            print_string("out of memory\n");
        } else {
            print_string("allocated frame at 0x");
            print_uint64_hex(f);
            print_char('\n');
        }
    } else if (strncmp(cmd, "free ", 5) == 0) {
        /* parse hex address from cmd+5, call free_frame */
    } else if (strcmp(cmd, "frames") == 0) {
        print_string("free frames: ");
        print_uint64_dec(frames_free_count());
        print_char('\n');
    } else if (strcmp(cmd, "maptest") == 0) {
        cmd_maptest();
    } else if (strcmp(cmd, "heap") == 0) {
        void* a = kmalloc(64);
        void* b = kmalloc(128);
        print_string("kmalloc(64)  -> 0x");
        print_uint64_hex((uint64_t)a);
        print_char('\n');
        print_string("kmalloc(128) -> 0x");
        print_uint64_hex((uint64_t)b);
        print_char('\n');

        if (a && b) {
            *(uint64_t*)a = 0xCAFEBABE;
            *(uint64_t*)b = 0xDEADBEEF;
            print_string("Read back a: 0x");
            print_uint64_hex(*(uint64_t*)a);
            print_char('\n');
            print_string("Read back b: 0x");
            print_uint64_hex(*(uint64_t*)b);
            print_char('\n');
        } else {
            print_string("kmalloc failed\n");
        }
    } else if (strcmp(cmd, "splittest") == 0){
        cmd_splittest();
    }else {
        print_string("unknown: ");
        print_string(cmd);
        print_char('\n');
    }
}
