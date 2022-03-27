#include "timer.h"

struct timer_task *timer_task_list = NULL;

void add_timer (void (*callback) (), void *data, unsigned int after) {

    unsigned long c_time;
    struct timer_task *new_task;
    
    c_time = time();
    
    /* Create nww timer task */
    new_task = malloc(sizeof(struct timer_task));
    new_task->execute_time = c_time + after;
    new_task->callback     = callback;
    new_task->data         = data;
    new_task->next_task    = NULL;

    /* First task */
    if (timer_task_list == NULL)
    {

        timer_task_list = new_task;
        
        /* Set timeout first */
        set_timeout(after);

        /* Enable core timer interrupt */
        set_timer_interrupt(true);

    }
    else /* Insert into list */
    {

        struct timer_task *prev;
        struct timer_task *c;
        prev = NULL;
        c    = timer_task_list;

        while (c != NULL)
        {

            if (c->execute_time > new_task->execute_time)
            {
                /* Position found */
                break;
            }

            prev = c;
            c = c->next_task;
        }

        /* Insert */
        if (prev != NULL)
        {
            new_task->next_task = prev->next_task;
            prev->next_task = new_task;
        }
        else
        {
            /* Insert at head */
            new_task->next_task = timer_task_list;
            timer_task_list = new_task;

            /* Update timeout */
            set_timeout(after);
        }

    }

    /* Print prompt */
    uart_puts("[ ");
    uart_putu(c_time);
    uart_puts(" secs ] Timer task will be execute ");
    uart_putu(after);
    uart_puts(" secs later. (at ");
    uart_putu(new_task->execute_time);
    uart_puts(" secs)\n");

    return;
}

void set_timeout (unsigned int seconds) {

    // Set next expire time
    asm volatile ("mrs x2, cntfrq_el0");
    asm volatile ("mul x1, x2, %0" :: "r"(seconds));
    asm volatile ("msr cntp_tval_el0, x1");

    return;
}

unsigned long time () {

    unsigned long cntpct_el0;
    unsigned long cntfrq_el0;
    unsigned long time_in_sec;

    asm volatile("mrs %0,  cntpct_el0" : "=r"(cntpct_el0) : );
    asm volatile("mrs %0,  cntfrq_el0" : "=r"(cntfrq_el0) : );

    time_in_sec = cntpct_el0 / cntfrq_el0;

    return time_in_sec;
}

void print_time () {

    unsigned long c_time = time();

    uart_puts("[Current time] ");
    uart_putu(c_time);
    uart_puts("secs\n");

    return;
}