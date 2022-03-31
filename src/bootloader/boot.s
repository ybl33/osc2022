
.section ".text.boot"

.global _start

__relocate_start = 0x60000;

_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbnz    x1, hang

    // compute addr of relocated main
    ldr     x1, =main
    ldr     x2, =__prog_start
    ldr     x3, =__relocate_start
    sub     x1, x1, x2
    add     x1, x1, x3
    // set top of stack
    mov     sp, x3

relocate:
    // relocate the program
    ldr     x5, =__prog_size
    cbz     x5, run_main
relocate_loop:  
    ldr     x4, [x2], #8
    str     x4, [x3], #8
    sub     x5, x5, #1
    cbnz    x5, relocate_loop

clear_bss:
    ldr     x2, =__prog_start
    ldr     x3, =__relocate_start
    ldr     x4, =__bss_start
    sub     x4, x4, x2
    add     x4, x4, x3
    ldr     x5, =__bss_size
    cbz     x5, run_main
clear_bss_loop:
    // clear bss segment
    str     xzr, [x4], #8
    sub     x5, x5, #1
    cbnz    x5, clear_bss_loop

run_main:
    // jump to C code, should not return
    br      x1
    b       run_main

hang: 
    // cpu id > 0, stop
    wfe
    b       hang