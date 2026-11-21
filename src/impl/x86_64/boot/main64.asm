global long_mode_start
extern kernel_main
extern multiboot_ptr

section .text
bits 64

long_mode_start:
    mov rax, higher_half_entry
    jmp rax

higher_half_entry:
    mov qword [page_table_l4 - KERNEL_VMA + 0*8], 0
    
    mov rax, cr3
    mov cr3, rax

    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax       ; zero everything

    mov rsp, stack_top

    mov rdi, [rel multiboot_ptr]
    mov rax, 0xFFFF800000000000
    add rdi, rax

    call kernel_main ; call the c program because assembly is sh*t
    hlt              ; end program