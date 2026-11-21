global start
extern long_mode_start

%define KERNEL_VMA 0xFFFF800000000000

section .boot.bss nobits
global multiboot_ptr
multiboot_ptr: resq 1

alignb 4096
global page_table_l4
page_table_l4: resb 4096
page_table_l3_low: resb 4096
page_table_l2_low: resb 4096
page_table_l3_high: resb 4096
page_table_l2_high: resb 4096

section .bss
alignb 4096
stack_bottom: resb 4096 * 4
global stack_top
stack_top:

section .text
bits 32

start:
    mov [multiboot_ptr], ebx
    mov esp, stack_top - KERNEL_VMA
    call check_multiboot
    call check_cpuid
    call check_long_mode
    call setub_page_tables
    call enable_paging

    lgdt [gdt64.pointer]        ; gdt64 is in .boot.data (low), no subtraction

    jmp gdt64.code_segment:long_mode_start    ; far jump to 64-bit
    hlt

check_multiboot:
    cmp eax, 0x36d76289
    jne .no_multiboot
    ret
.no_multiboot:
    mov al, "M"
    jmp error

error:
    mov dword [0xb8000], 0x4f524f45
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x4f204f20
    mov byte  [0xb800c], al
    mov dword [0xb8010], 0x00000000
    hlt

check_cpuid:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 0x200000
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    xor eax, ecx
    jz .no_cpuid
    ret
.no_cpuid:
    mov al, "C"
    jmp error

check_long_mode:
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode

    mov eax, 0x80000001
    cpuid
    test edx, 0x20000000
    jz .no_long_mode
    ret
.no_long_mode:
    mov al, "L"
    jmp error

setub_page_tables:
    cld
    mov edi, page_table_l4
    mov ecx, 5 * 1024
    xor eax, eax
    rep stosd

    mov eax, page_table_l3_low
    or eax, 0b11
    mov [page_table_l4 + 0*8], eax
    mov dword [page_table_l4 + 0*8 + 4], 0

    mov eax, page_table_l3_high
    or eax, 0b11
    mov [page_table_l4 + 256*8], eax
    mov dword [page_table_l4 + 256*8 + 4], 0

    mov eax, page_table_l2_low
    or eax, 0b11
    mov [page_table_l3_low + 0*8], eax
    mov dword [page_table_l3_low + 0*8 + 4], 0

    mov eax, page_table_l2_high
    or eax, 0b11
    mov [page_table_l3_high + 0*8], eax
    mov dword [page_table_l3_high + 0*8 + 4], 0

    mov ecx, 0
.loop:
    mov eax, 0x200000
    mul ecx
    or eax, 0b10000011
    mov [page_table_l2_low + ecx*8], eax
    mov dword [page_table_l2_low + ecx*8 + 4], 0
    mov [page_table_l2_high + ecx*8], eax
    mov dword [page_table_l2_high + ecx*8 + 4], 0
    inc ecx
    cmp ecx, 512
    jne .loop
    ret

enable_paging:
    mov eax, cr4
    or eax, 1 << 5              ; PAE first
    mov cr4, eax

    mov eax, page_table_l4
    mov cr3, eax                ; then CR3

    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8              ; LME
    wrmsr

    mov eax, cr0
    or eax, 1 << 31             ; PG
    mov cr0, eax
    ret


section .boot.data
gdt64:
    dq 0
.code_segment: equ $ - gdt64
    dq (1 << 43)|(1 << 44)|(1 << 47)|(1 << 53)
.pointer:
    dw $ - gdt64 - 1
    dq gdt64