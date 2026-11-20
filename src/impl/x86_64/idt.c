#include "x86_64/panic.h"
#include "print.h"
#include "x86_64/serial.h"

#define IDT_IRQ0_TIMER 0x20
#define IDT_IRQ1_KEYBOARD 0x21
#define IDT_PAGE_FAULT 0x0E

#define IDT_GATE_PRESENT (1 << 7)
#define IDT_GATE_DPL0 (0b00 << 5)
#define IDT_GATE_DPL1 (0b01 << 5)
#define IDT_GATE_DPL2 (0b10 << 5)
#define IDT_GATE_DPL3 (0b11 << 5)
#define IDT_GATE_TYPE_INTERRUPT 0xE

#define IDT_ENTRY_TYPE_INTERRUPT (IDT_GATE_PRESENT | IDT_GATE_DPL0 | IDT_GATE_TYPE_INTERRUPT)

#define PF_PRESENT   (1 << 0)
#define PF_WRITE     (1 << 1)
#define PF_USER      (1 << 2)
#define PF_RESERVED  (1 << 3)
#define PF_INSTR     (1 << 4)

void idt_handler_unhandled(void) {
    panic("unhandled interrupt");
}

extern void idt_handler_keyboard_wrapped();
extern void idt_page_fault_wrapped(void);

#include <timer.h>
#include <stdint.h>
#include <stddef.h>
#include "gdt.h"
#include "idt.h"
#include "x86_64/pic.h"

struct IdtEntry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));

struct IdtPtr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

struct IdtEntry idt[256] __attribute__((aligned(16)));

struct IdtPtr idt_ptr;

void (*idt_handler_keyboard_user)();
void (*idt_handler_timer_user)(void) = NULL;

extern void idt_load(struct IdtPtr* idt_ptr);
extern void idt_handler_timer_wrapped(void);

void idt_handler_timer(void){
    if (idt_handler_timer_user != NULL) {
        idt_handler_timer_user();
    }
    pic_eoi_master();
}

void idt_set_handler_timer(void (*handler)(void)) {
    idt_handler_timer_user = handler;
}

void idt_set_entry(uint8_t vector, uint64_t isr_addr, uint16_t selector, uint8_t type) {
    idt[vector] = (struct IdtEntry) {
        .offset_low = (uint16_t) (isr_addr >> 0),
        .selector = selector,
        .ist = 0,
        .type = type,
        .offset_mid = (uint16_t) (isr_addr >> 16),
        .offset_high = (uint16_t) (isr_addr >> 32),
        .reserved = 0,

    };
}

void idt_handler_keyboard() {
    if (idt_handler_keyboard_user != NULL) {
        idt_handler_keyboard_user();
    }

    pic_eoi_master();
}

void idt_init() {
    pic_remap();

    idt_set_entry(IDT_PAGE_FAULT,    (uint64_t) idt_page_fault_wrapped, GDT_SELECTOR_CS_KERNEL, IDT_ENTRY_TYPE_INTERRUPT);
    idt_set_entry(IDT_IRQ0_TIMER,    (uint64_t) idt_handler_timer_wrapped,      GDT_SELECTOR_CS_KERNEL, IDT_ENTRY_TYPE_INTERRUPT);
    idt_set_entry(IDT_IRQ1_KEYBOARD, (uint64_t) idt_handler_keyboard_wrapped,   GDT_SELECTOR_CS_KERNEL, IDT_ENTRY_TYPE_INTERRUPT);

    idt_ptr.limit = (sizeof(struct IdtEntry) * 256) - 1;
    idt_ptr.base = (uint64_t) &idt;
    idt_load(&idt_ptr);
}

void idt_set_handler_keyboard(void (*handler)()){
    idt_handler_keyboard_user = handler;
}

void idt_page_fault(uint64_t *error_code) {
    uint64_t cr2;
    asm volatile ("mov %%cr2, %0" : "=r"(cr2));

    uint64_t ec = *error_code;

    char msg[128];
    int  i = 0;

    const char *prefix = "page fault at 0x";
    while (*prefix) msg[i++] = *prefix++;

    int started = 0;
    for (int shift = 60; shift >= 0; shift -= 4) {
        uint8_t nib = (cr2 >> shift) & 0xF;
        if (nib || started || shift == 0) {
            started = 1;
            msg[i++] = (nib < 10) ? ('0' + nib) : ('a' + nib - 10);
        }
    }

    const char *p;
    p = " ("; while (*p) msg[i++] = *p++;
    p = (ec & PF_WRITE) ? "write" : "read";  while (*p) msg[i++] = *p++;
    p = (ec & PF_USER)  ? ", user" : ", kernel"; while (*p) msg[i++] = *p++;
    if (ec & PF_PRESENT)  { p = ", protection"; while (*p) msg[i++] = *p++; }
    else                  { p = ", not-present"; while (*p) msg[i++] = *p++; }
    if (ec & PF_RESERVED) { p = ", reserved-bit"; while (*p) msg[i++] = *p++; }
    if (ec & PF_INSTR)    { p = ", instruction-fetch"; while (*p) msg[i++] = *p++; }
    p = ")"; while (*p) msg[i++] = *p++;
    msg[i] = '\0';

    panic(msg);
}