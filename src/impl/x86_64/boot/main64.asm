global long_mode_start
global higher_half_entry
extern kernel_main
extern multiboot_ptr
extern stack_top

%define KERNEL_VMA 0xFFFF800000000000

section .boot.text
bits 64

long_mode_start:
    movabs rax, higher_half_entry
    jmp rax

section .text
higher_half_entry:
    ; The stack is in the higher-half .bss mapping.
    movabs rsp, stack_top

    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; The identity map stays active for code that dereferences physical addresses.
    movabs rax, multiboot_ptr
    mov rdi, [rax]
    movabs rax, KERNEL_VMA
    add rdi, rax

    movabs rax, kernel_main
    call rax

    hlt