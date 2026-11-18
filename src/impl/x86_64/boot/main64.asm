global long_mode_start
extern kernel_main

section .text
bits 64

long_mode_start:
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax       ; zero everything

    call kernel_main ; call the c program because assembly is sh*t
    hlt              ; end program