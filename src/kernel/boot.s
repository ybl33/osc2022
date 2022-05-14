
.section ".text.boot"

.global _start

_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, 3
    // cpu id != 0, stop
    cbnz    x1, hang

get_current_EL:
    // current exception level is in CurrentEL[3:2]
    mrs     x1, CurrentEL
    and     x1, x1, 12
    lsr     x1, x1, 2
    // check whether runing at EL2
    cmp     x1, 2
    bne     hang
    // switch from EL2 to EL1
    bl      from_EL2_to_EL1

set_exception_vector_table:
    adr     x1, exception_vector_table
    msr     vbar_el1, x1

set_sp:
    // set top of stack just before our code (stack grows to a lower address per AAPCS64)
    ldr     x1, =_start
    mov     sp, x1

clear_bss:
    // clear bss
    ldr     x1, =__bss_start
    ldr     x2, =__bss_size
clear_bss_loop:
    cbz     x2, run_main
    str     xzr, [x1], 8
    sub     x2, x2, 1
    cbnz    x2, clear_bss_loop

run_main:
    // jump to kernel, should not return
    bl      main
    // for failsafe, halt this core too
    b       hang

hang:  
    wfe
    b       hang


// Execption
// save general registers to stack
.macro save_all
    sub     sp, sp, 16 * 18
    // saving general registers
    stp     x0, x1, [sp ,16 * 0]
    stp     x2, x3, [sp ,16 * 1]
    stp     x4, x5, [sp ,16 * 2]
    stp     x6, x7, [sp ,16 * 3]
    stp     x8, x9, [sp ,16 * 4]
    stp     x10, x11, [sp ,16 * 5]
    stp     x12, x13, [sp ,16 * 6]
    stp     x14, x15, [sp ,16 * 7]
    stp     x16, x17, [sp ,16 * 8]
    stp     x18, x19, [sp ,16 * 9]
    stp     x20, x21, [sp ,16 * 10]
    stp     x22, x23, [sp ,16 * 11]
    stp     x24, x25, [sp ,16 * 12]
    stp     x26, x27, [sp ,16 * 13]
    stp     x28, x29, [sp ,16 * 14]
    str     x30, [sp ,16 * 15]

    // saving el registers
    mrs     x0, SPSR_EL1
    str     x0, [sp, 16 * 15 + 8]
    mrs     x0, ELR_EL1
    mrs     x1, SP_EL0
    stp     x0, x1, [sp, 16 * 16]
.endm

// load general registers from stack
.macro load_all
    // load el registers
    ldr     x0, [sp, 16 * 15 + 8]
    msr     SPSR_EL1, x0
    ldp     x0, x1, [sp, 16 * 16]
    msr     ELR_EL1, x0
    msr     SP_EL0, x1

    // load general registers
    ldp     x0, x1, [sp ,16 * 0]
    ldp     x2, x3, [sp ,16 * 1]
    ldp     x4, x5, [sp ,16 * 2]
    ldp     x6, x7, [sp ,16 * 3]
    ldp     x8, x9, [sp ,16 * 4]
    ldp     x10, x11, [sp ,16 * 5]
    ldp     x12, x13, [sp ,16 * 6]
    ldp     x14, x15, [sp ,16 * 7]
    ldp     x16, x17, [sp ,16 * 8]
    ldp     x18, x19, [sp ,16 * 9]
    ldp     x20, x21, [sp ,16 * 10]
    ldp     x22, x23, [sp ,16 * 11]
    ldp     x24, x25, [sp ,16 * 12]
    ldp     x26, x27, [sp ,16 * 13]
    ldp     x28, x29, [sp ,16 * 14]
    ldr     x30, [sp, 16 * 15]

    add     sp, sp, 16 * 18
.endm

syn:
    save_all
    mov x0, sp
    bl syn_handler
    load_all
    eret

irq:
    save_all
    mov x0, sp
    bl irq_handler
    load_all
    eret

unsupported:
    save_all
    bl undefined_handler
    load_all
    eret

// vector table should be aligned to 0x800
.align 11 
.global exception_vector_table
exception_vector_table:
    // branch to a handler function.
    b unsupported 
    // entry size is 0x80, .align will pad 0
    .align 7 
    b unsupported 
    .align 7
    b unsupported 
    .align 7
    b unsupported 
    .align 7

    b hang 
    .align 7
    b irq 
    .align 7
    b unsupported 
    .align 7
    b unsupported 
    .align 7

    b syn 
    .align 7
    b irq 
    .align 7
    b unsupported 
    .align 7
    b unsupported 
    .align 7

    b unsupported 
    .align 7
    b unsupported 
    .align 7
    b unsupported 
    .align 7
    b unsupported 
    .align 7

.global from_EL2_to_EL1
from_EL2_to_EL1:    
    // set EL1 as AArch64
    mov     x1, (1 << 31)
    msr     hcr_el2, x1
    // set spsr as (EL1h, and irq enabled) 
    mov     x2, 0x345
    msr     spsr_el2, x2
    // set return address with link register
    msr     elr_el2, lr
    eret

.global from_EL1_to_EL0
from_EL1_to_EL0:
    msr     elr_el1, x0
    msr     sp_el0, x1
    mov     sp, x2
    mov     x1, 0x340
    msr     spsr_el1, x1
    eret

.global switch_to
switch_to:
    stp x19, x20, [x0, 16 * 0]
    stp x21, x22, [x0, 16 * 1]
    stp x23, x24, [x0, 16 * 2]
    stp x25, x26, [x0, 16 * 3]
    stp x27, x28, [x0, 16 * 4]
    stp fp, lr, [x0, 16 * 5]
    mov x9, sp
    str x9, [x0, 16 * 6]

    ldp x19, x20, [x1, 16 * 0]
    ldp x21, x22, [x1, 16 * 1]
    ldp x23, x24, [x1, 16 * 2]
    ldp x25, x26, [x1, 16 * 3]
    ldp x27, x28, [x1, 16 * 4]
    ldp fp, lr, [x1, 16 * 5]
    ldr x9, [x1, 16 * 6]
    mov sp,  x9
    msr tpidr_el1, x2
    ret

.global store_context
store_context:
    stp x19, x20, [x0, 16 * 0]
    stp x21, x22, [x0, 16 * 1]
    stp x23, x24, [x0, 16 * 2]
    stp x25, x26, [x0, 16 * 3]
    stp x27, x28, [x0, 16 * 4]
    stp fp, lr, [x0, 16 * 5]
    mov x9, sp
    str x9, [x0, 16 * 6]
    ret

.global load_context
load_context:
    ldp x19, x20, [x0, 16 * 0]
    ldp x21, x22, [x0, 16 * 1]
    ldp x23, x24, [x0, 16 * 2]
    ldp x25, x26, [x0, 16 * 3]
    ldp x27, x28, [x0, 16 * 4]
    ldp fp, lr, [x0, 16 * 5]
    ldr x9, [x0, 16 * 6]
    mov sp,  x9
    ret


// system calls
.global sys_get_pid
sys_get_pid:
    mov x8, 0
    svc 0
    ret

.global sys_uart_read
sys_uart_read:
    mov x8, 1
    svc 0
    ret

.global sys_uart_write
sys_uart_write:
    mov x8, 2
    svc 0
    ret

.global sys_exec
sys_exec:
    mov x8, 3
    svc 0
    ret

.global sys_fork
sys_fork:
    mov x8, 4
    svc 0
    ret

.global sys_exit
sys_exit:
    mov x8, 5
    svc 0
    ret

.global sys_mbox_call
sys_mbox_call:
    mov x8, 6
    svc 0
    ret

.global sys_kill
sys_kill:
    mov x8, 7
    svc 0
    ret

.global sys_ret
sys_ret:
    mov x8, 10
    svc 0
    ret