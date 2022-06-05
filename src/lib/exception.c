#include "exception.h"

/* Exception list */
struct exception_task *exception_task_list = NULL;

void log_el1_status () {
    
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

unsigned int interrupt_lock = 0;

void set_interrupt (bool enable) {

    if (enable)
    {
        if (interrupt_lock > 0)
        {
            interrupt_lock--;
        }
        
        if (interrupt_lock == 0)
        {
            asm volatile ("msr DAIFClr, 0xf");
        }
        
    }
    else
    {
        interrupt_lock++;
        asm volatile ("msr DAIFSet, 0xf");
    }

    return;
}

void set_timer_interrupt (bool enable) {

    /* Needs to set timeout first */

    if (enable)
    {
        // Enable
        asm volatile ("mov x0, 1");
        asm volatile ("msr cntp_ctl_el0, x0");
        // Unmask timer interrupt
        asm volatile ("mov x0, 2");
        asm volatile ("ldr x1, =0xFFFF000040000040");
        asm volatile ("str w0, [x1]");
    }
    else
    {
        // mask timer interrupt
        asm volatile ("mov x0, 0");
        asm volatile ("ldr x1, =0xFFFF000040000040");
        asm volatile ("str w0, [x1]");
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

void syn_handler (trap_frame_t* trap_frame) {

    unsigned int svc = trap_frame->regs[8];
    unsigned int ret;

    set_interrupt(true);

    switch (svc) {
        case 0:
            ret = get_pid();
            trap_frame->regs[0] = ret;
            break;
        case 1:
            ret = uart_read((char *)trap_frame->regs[0], trap_frame->regs[1]);
            trap_frame->regs[0] = ret;
            break;
        case 2:
            ret = uart_write((char *)trap_frame->regs[0], trap_frame->regs[1]);
            trap_frame->regs[0] = ret;
            break;
        case 3:
            ret = exec(trap_frame, (char *)trap_frame->regs[0], (char **)trap_frame->regs[1]);
            trap_frame->regs[0] = ret;
            break;
        case 4:
            ret = thread_fork(trap_frame);
            trap_frame->regs[0] = ret;
            break;
        case 5:
            exit();
            break;
        case 6:
            ret = mbox_call((mail_t *)(trap_frame->regs[1]), (unsigned char)trap_frame->regs[0]);
            trap_frame->regs[0] = ret;
            break;
        case 7:
            kill(trap_frame->regs[0]);
            break;
        case 8:
            thread_signal_register(trap_frame->regs[0], (void *)trap_frame->regs[1]);
            break;
        case 9:
            thread_signal_kill(trap_frame->regs[0], trap_frame->regs[1]);
            break;
        case 10:
            thread_signal_return();
            break;
        default:
            uart_puts("[syn_handler] Unsupported svc: ");
            uart_puth(svc);
            uart_puts("\n");
            break;
    }

    return;
}

void irq_handler (trap_frame_t* trap_frame) {

    /* IRQ Source */
    unsigned int is_timer_irq;
    unsigned int is_uart_irq;

    /* New task */
    struct exception_task *new_task = NULL;
    unsigned int priority = 0;
    void (*handler) ()    = NULL;

    /* Flags */
    bool is_first      = false;
    bool is_preemption = false;
    bool done_all_task;

    /* Enter critical section */
    set_interrupt(false);

    is_timer_irq = mmio_get(CORE_0_IRQ_SOURCE) & CORE_IRQ_CNTPNS;
    is_uart_irq  = mmio_get(IRQ_1_PENDING) & IRQ_1_AUX_INT;

    /* Analysis IRQ source */
    if (is_timer_irq)
    {

        handler  = timer_irq_handler;
        priority = TIMER_IRQ_PRIO;

        // Disable core timer interrupt until handler finish its work
        set_timer_interrupt(false);
    }
    else if (is_uart_irq)
    {
        handler  = uart_irq_handler;
        priority = UART_IRQ_PRIO;

        // Disable aux interrupt until handler finish its work
        set_aux_int(false);
    }

    /* Add handler into exception task list */
    if (handler != NULL)
    {

        /* Construct new task */
        new_task = kmalloc(sizeof(struct exception_task));
        new_task->priority  = priority;
        new_task->handler   = handler;
        new_task->next_task = NULL;

        if (exception_task_list == NULL || (exception_task_list->priority < new_task->priority))
        {

            /* No pending task or preemption */
            if (exception_task_list == NULL)
            {
                is_first = true;
            }
            else
            {
                is_preemption = true;
            }

            new_task->next_task = exception_task_list;
            exception_task_list = new_task;

        }
        else
        {

            struct exception_task *prev;
            struct exception_task *c;

            prev = NULL;
            c    = exception_task_list;

            while (c != NULL)
            {
                if (c->priority < new_task->priority)
                {
                    break;
                }

                prev = c;
                c = c->next_task;
            }

            /* Insert */
            new_task->next_task = prev->next_task;
            prev->next_task = new_task;

        }

    }

    /* Exit critical section */
    set_interrupt(true);

    /* First exception, execute handler immediately */
    if (is_first)
    {

        done_all_task = false;

        struct exception_task *curr_task;
        void (*next_task) () = NULL;

        while (!done_all_task) {

            /* Do the rest */
            set_interrupt(false);
            curr_task = exception_task_list;
            exception_task_list = exception_task_list->next_task;
            done_all_task       = (exception_task_list == NULL);

            if (!done_all_task)
            {
                next_task = exception_task_list->handler;
            }
            
            kfree(curr_task);

            set_interrupt(true);
            
            /* Execute handler with interrupt enabled */
            handler();

            handler = next_task;


        };

    }

    /* Preemption, execute handler immediately */
    if (is_preemption)
    {
        
        handler();

        set_interrupt(false);
        exception_task_list = exception_task_list->next_task;
        set_interrupt(true);
        
    }   

    return;
}

void undefined_handler () {

    uart_puts("Undefined exception occur.\n");

    return;
}

void uart_irq_handler () {

    unsigned int is_rx_irq;
    unsigned int is_tx_irq;

    // Disable aux interrupt
    set_aux_int(false);
    
    is_rx_irq = mmio_get(AUX_MU_IIR_REG) & 0x4; // Receiver holds valid byte
    is_tx_irq = mmio_get(AUX_MU_IIR_REG) & 0x2; // Transmit holding register empty

    if (is_rx_irq)
    {

        while (uart_rx_valid())
        {
            read_buffer[read_head] = uart_get();
            read_head = (read_head + 1) & (READ_BUF_SIZE - 1);
        }
        
    }
    else if (is_tx_irq)
    {

        while (write_head != write_tail)
        {
            uart_put(write_buffer[write_tail]);
            write_tail = (write_tail + 1) & (WRITE_BUF_SIZE - 1);
        }

        /* If write done, disable TX interrupt */
        set_uart_tx_int(false);
        
    }
    
    set_aux_int(true);

    return;
}

void timer_irq_handler () {

    unsigned long c_time, after;
    struct timer_task *curr_task;

    c_time = time();

    /* Print prompt */
    // uart_puts("\n\n");
    // uart_puts("[ ");
    // uart_putu(c_time);
    // uart_puts(" secs ] Timer irq occur, execute pre-scheduled task.");
    // uart_puts("\n\n");

    /* Do callback */
    curr_task = timer_task_list;
    timer_task_list->callback(timer_task_list->data);
    timer_task_list = timer_task_list->next_task;

    /* Update timeout or disable timer interrupt */
    if (timer_task_list == NULL)
    {
        /* No task to do, disable timer interrupt */
        set_timer_interrupt(false);
    }
    else
    {
        if (timer_task_list->execute_time > c_time)
        {
            after = timer_task_list->execute_time - c_time;
        }
        else
        {
            after = 0;
        }

        /* Set next timeout */
        set_timeout_by_ticks(after);

        /* Enable the timer interrupt */
        set_timer_interrupt(true);

    }

    kfree(curr_task);

    thread_schedule();

    return;
}