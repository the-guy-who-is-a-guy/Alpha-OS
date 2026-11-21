# Copilot Instructions

## Project Overview

Alpha OS is a small 64-bit x86 kernel and bootable ISO built from freestanding C and NASM assembly. It starts in `src/impl/x86_64/boot/main.asm`, enters long mode, switches to a higher-half virtual address layout, and calls `kernel_main()` in `src/impl/kernel/main.c`.

The repository has no hosted application or library build. The main deliverables are:

- `dist/x86_64/kernel.bin`: linked ELF kernel binary
- `dist/x86_64/kernel.iso`: bootable GRUB ISO
- `build/`: intermediate C and assembly objects
- `targets/x86_64/iso/`: ISO staging tree and GRUB configuration

## Build, Run, and Test

Prerequisites used directly by the Makefile are GCC, NASM, GNU `ld`, and `grub-mkrescue` from a GRUB i386-pc installation.

- Build the kernel and ISO: `make build`
- Run the ISO in QEMU with serial output: `make test`
- Clean generated objects and distributions: `make clean`
- Rebuild, run, and clean: `make run`

`make run` invokes `sudo make build`, so do not assume a normal build requires elevated privileges. A direct rebuild/run sequence is:

```sh
make clean
make build
make test
```

There is no unit-test framework or lint target. `make test` boots the complete ISO in QEMU and is therefore the repository-level test. There is no single-test runner or test name option.

For a focused runtime check, boot with `make test`, then use the interactive shell commands. `maptest` exercises page mapping, `splittest` exercises splitting a 2 MiB huge page, `heap` exercises `kmalloc`, and `frames` reports frame allocator state. Use serial stdio for boot output and the shell prompt.

The Makefile discovers C and assembly sources dynamically under `src/impl/kernel` and `src/impl/x86_64`. C objects compile with `gcc -I src/intf -ffreestanding`; assembly objects compile as ELF64 with NASM. The linker script is `targets/x86_64/linker.ld`.

## High-Level Architecture

### Boot and Address Space

- `src/impl/x86_64/boot/header.asm` provides the Multiboot2 header.
- `src/impl/x86_64/boot/main.asm` performs protected-mode checks, constructs PML4/PDP/PD tables, enables PAE and long mode, and installs the GDT.
- `src/impl/x86_64/boot/main64.asm` switches to the higher-half entry, moves the stack, removes the low identity mapping, and calls `kernel_main`.
- `targets/x86_64/linker.ld` places `.boot.*` at the low 1 MiB region and the kernel at virtual base `0xFFFF800000000000`.

Keep the initial identity-mapped bootstrap and the higher-half kernel mapping distinct. Changes to the linker layout or virtual base require coordinated updates in the linker script and assembly address constants.

### Initialization and Runtime Subsystems

`kernel_main()` is the central initialization sequence:

1. Initialize VGA text output and serial.
2. Parse the Multiboot2 memory map.
3. Initialize the physical-frame bitmap.
4. Initialize the IDT, PIC, timer, keyboard, and heap.
5. Enable interrupts and enter the interactive shell loop.

Core subsystems live in `src/impl/x86_64` and `src/impl/kernel`; public interfaces live in `src/intf`.

- `multiboot2.c` extracts usable physical memory ranges.
- `falloc.c` provides a bitmap physical-frame allocator capped at 128 MiB.
- `vmm.c` walks PML4/PDP/PD/PT tables and splits 2 MiB huge pages when a 4 KiB mapping is needed.
- `heap.c` maps an initial 1 MiB of 16 MiB of bump-allocated kernel heap.
- `idt.c` and `idt_.asm` connect CPU/PIC interrupt vectors to C handlers.
- `timer.c`, `rtc.c`, `keyboard.c`, `ps2.c`, and `print.c` implement device and console functionality.
- `shell.c` is the command dispatcher and interactive line editor.

### Public Interface Layout

The `src/intf` tree mirrors subsystem names. Platform headers are under `src/intf/x86_64/`, while kernel-facing headers such as `vmm.h`, `falloc.h`, `heap.h`, `timer.h`, and `shell.h` are in `src/intf`. Compile/include changes must use these public headers rather than adding cross-directory implementation includes.

## Key Conventions

- C code is freestanding: no hosted C library assumptions. Use the repository implementations in `src/intf` and `src/impl/x86_64/string.c` instead of libc APIs.
- Public headers use `#pragma once` and integer types from `<stdint.h>`, `<stddef.h>`, or the repository's small `bool.h` interface.
- Preserve the kernel's higher-half virtual base and explicit physical/virtual address conversions. Page-table pointers and frame addresses are generally physical address values until explicitly converted or mapped.
- Physical allocations use `alloc_frame()`; `0` is the allocator's out-of-memory sentinel. Page mappings use `vmm_map_page(virt, phys, flags)` and page flags from `src/intf/vmm.h`.
- Interrupt handlers must preserve PIC EOI behavior and register callbacks through the IDT setter functions.
- Keep boot-stage `.boot.text`, `.boot.data`, and `.boot.bss` sections separate from normal higher-half sections in assembly and linker changes.
- Add shell commands in `handle_command()` in `src/impl/x86_64/shell.c`; keep command parsing consistent with the existing exact-match and `strncmp` argument forms.
- Runtime diagnostics should use `print_*` for VGA output and `serial_*` for QEMU/serial logging. `panic()` is used for fatal kernel conditions.
