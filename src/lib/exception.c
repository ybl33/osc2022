#include "exception.h"

void syn_handler () {

    unsigned long esr_el1;
    unsigned long elr_el1;
    unsigned long spsr_el1;

    // Get EL1 registers
    asm volatile("mrs %0,  esr_el1" : "=r"(esr_el1) : );
    asm volatile("mrs %0,  elr_el1" : "=r"(elr_el1) : );
    asm volatile("mrs %0, spsr_el1" : "=r"(spsr_el1) : );

    uart_puts("ESR_EL1 : 0x");
    uart_puth(esr_el1);
    uart_puts("\n");
    uart_puts("ELR_EL1 : 0x");
    uart_puth(elr_el1);
    uart_puts("\n");
    uart_puts("SPSR_EL1: 0x");
    uart_puth(spsr_el1);
    uart_puts("\n");
    uart_puts("----------------------------\n");

    return;
}

void irq_handler () {

    unsigned long c_time;

    // Set next expire time, 2 seconds later
    asm volatile ("mrs x0, cntfrq_el0");
    asm volatile ("lsl x0, x0, 1");
    asm volatile ("msr cntp_tval_el0, x0");

    // Get current time
    c_time = time();

    // Print prompt
    uart_puts("[IRQ Interrupt] HIHI I'm IRQ handler ^u^, current time is ");
    uart_putu(c_time);
    uart_puts(".\n");

    return;
}

void undefined_handler () {

    uart_puts("Undefined exception occur.\n");

    return;
}