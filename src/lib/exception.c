#include "exception.h"

void set_interrupt (bool enable) {

    if (enable)
    {
        asm volatile ("msr DAIFClr, 0xf");
    }
    else
    {
        asm volatile ("msr DAIFSet, 0xf");
    }

    return;
}

void set_timer_interrupt (bool enable) {

    if (enable)
    {
        // Enable
        asm volatile ("mov x0, 1");
        asm volatile ("msr cntp_ctl_el0, x0");
        // Init expire time, 1 second later
        asm volatile ("mrs x0, cntfrq_el0");
        asm volatile ("msr cntp_tval_el0, x0");
        // Unmask timer interrupt
        asm volatile ("mov x0, 2");
        asm volatile ("ldr x1, =0x40000040");
        asm volatile ("str w0, [x1]");
    }
    else
    {
        asm volatile ("mov x0, 0");
        asm volatile ("msr cntp_ctl_el0, x0");  
    }

    return;
}

void set_aux_int (bool enable) {
    
    if (enable)
    {
        /* Set AUX INT */
        mmio_put(IRQs_1_ENABLE, IRQ_1_AUX_INT);
    }
    else
    {
        mmio_put(IRQs_1_DISABLE, IRQ_1_AUX_INT);
    }

    return;
}

void set_timeout (unsigned int seconds) {

    // Set next expire time
    asm volatile ("mrs x2, cntfrq_el0");
    asm volatile ("mul x1, x2, %0" :: "r"(seconds));
    asm volatile ("msr cntp_tval_el0, x1");

    return;
}

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

    unsigned int is_timer_irq;
    unsigned int is_uart_irq;

    set_interrupt(false);

    is_timer_irq = mmio_get(CORE_0_IRQ_SOURCE) & CORE_IRQ_CNTPNS;
    is_uart_irq  = mmio_get(IRQ_1_PENDING) & IRQ_1_AUX_INT;

    if (is_timer_irq)
    {
        timer_irq_handler();
    }
    else if (is_uart_irq)
    {
        uart_irq_handler();
    }

    set_interrupt(true);

    return;
}

void undefined_handler () {

    // uart_puts("Undefined exception occur.\n");

    return;
}

void uart_irq_handler () {

    unsigned int is_rx_irq;
    unsigned int is_tx_irq;
    char c;

    set_aux_int(false);

    is_rx_irq = mmio_get(AUX_MU_IIR_REG) & 0x4; // Receiver holds valid byte
    is_tx_irq = mmio_get(AUX_MU_IIR_REG) & 0x2; // Transmit holding register empty

    if (is_rx_irq)
    {
        c = uart_get();
        read_buffer[read_head] = c;
        read_head = (read_head + 1) & (READ_BUF_SIZE - 1);


        asyn_uart_put(c);
        uart_puts("[RX IRQ] Received: ");
        uart_put(c);
        uart_put('\n');
    }
    else if (is_tx_irq)
    {

        c = write_buffer[write_tail];
        write_tail = (write_tail + 1) & (WRITE_BUF_SIZE - 1);

        uart_puts("[TX IRQ] Send: ");
        uart_put(c);
        uart_puts("\n------------------\n");

        /* If write done, disable TX interrupt */
        if (write_head == write_tail)
        {
            set_uart_tx_int(false);
        }

    }
    
    set_aux_int(true);

    return;
}

void timer_irq_handler () {

    unsigned long c_time;

    // Set next expire time
    set_timeout(3);

    // Get current time
    c_time = time();

    // Print prompt
    uart_puts("[IRQ Interrupt] HIHI I'm IRQ handler ^u^, current time is ");
    uart_putu(c_time);
    uart_puts(".\n");

    return;
}