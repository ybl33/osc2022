#include "thread.h"

extern void store_context(thread_context_t *context);
extern void load_context(thread_context_t *context);
extern void switch_to(thread_context_t *curr_context, thread_context_t *next_context, thread_t *next_thread);
extern void from_EL1_to_EL0(unsigned long prog, unsigned long user_sp, unsigned long kernel_sp);
thread_queue_t *idle_queue = NULL;
thread_queue_t *wait_queue = NULL;
bool pid_inuse[THREAD_MAX_NUM];

extern int sys_get_pid();
extern int sys_fork();
extern int sys_exit();
extern void sys_ret();

void thread_queue_init (thread_queue_t* queue) {

    queue->front = NULL;
    queue->tail  = NULL;

    return;
}

thread_t* thread_dequeue (thread_queue_t* queue) {

    thread_t* node = NULL;

    if (queue->front != NULL) 
    {
        node = queue->front;

        if (queue->front->next == NULL)
        {
            queue->tail = NULL;
        }

        queue->front = queue->front->next;

        node->next = NULL;
    }

    return node;
}

void thread_enqueue (thread_queue_t* queue, thread_t* node) {

    if (queue->tail == NULL)
    {
        queue->front = node;
    }
    else
    {
        queue->tail->next = node;
    }
    
    queue->tail = node;

    node->next = NULL;

    return;
}

void _thread_init () {

    thread_t* kernel_thread;

    idle_queue = kmalloc(sizeof(thread_queue_t));
    wait_queue = kmalloc(sizeof(thread_queue_t));

    thread_queue_init(idle_queue);
    thread_queue_init(wait_queue);

    for (int i = 0; i < THREAD_MAX_NUM; i++)
    {
        pid_inuse[i] = false;
    }

    // Create kernel thread
    kernel_thread = kmalloc(sizeof(thread_t));
    kernel_thread->status = THREAD_WAIT;
    kernel_thread->pid    = 0;

    for (int i = 0; i < THREAD_MAX_SIG_NUM; i++)
    {
        kernel_thread->signal_handlers[i] = NULL;
        kernel_thread->signal_num[i] = 0;
    }
    
    thread_enqueue(wait_queue, kernel_thread);
    asm volatile ("msr tpidr_el1, %0" : : "r"(kernel_thread));

    pid_inuse[0] = true;

    return;
}

pid_t _get_new_pid () {

    pid_t pid;

    for (pid = 0; pid < THREAD_MAX_NUM; pid++) 
    {
        if (pid_inuse[pid] == 0)
        {
            pid_inuse[pid] = true;
            return pid;
        }
    }

    return -1;
}

void _thread_timer_task () {

    unsigned long cntfrq_el0;

    asm volatile("mrs %0,  cntfrq_el0" : "=r"(cntfrq_el0) : );

    add_timer(_thread_timer_task, NULL, cntfrq_el0 >> 5);

    return;
}

thread_t* get_current_thread () {

    thread_t* curr_thread;

    asm volatile ("mrs %0, tpidr_el1" : "=r"(curr_thread));

    return curr_thread;
}

pid_t thread_get_pid () {

    pid_t pid;
    thread_t* curr_thread;

    curr_thread = get_current_thread();
    pid = curr_thread->pid;

    return pid;
}

thread_t* thread_create ( void (*func) () ) {

    thread_t *new_thread = NULL;
    pid_t pid;

    if (idle_queue == NULL)
    {
        _thread_init();
    }

    pid = _get_new_pid();

    if (pid != -1)
    {
        new_thread = kmalloc(sizeof(thread_t));

        new_thread->pid          = pid;
        new_thread->status       = THREAD_IDLE;
        new_thread->code_addr    = (unsigned long)func;
        new_thread->kernel_stack = (unsigned long)alloc_pages(4);
        new_thread->user_stack   = (unsigned long)alloc_pages(4);

        new_thread->context.lr = (unsigned long)func;
        new_thread->context.fp = (unsigned long)new_thread->user_stack + THREAD_STACK_SIZE;
        new_thread->context.sp = (unsigned long)new_thread->user_stack + THREAD_STACK_SIZE;
        new_thread->next       = NULL;

        for (int i = 0; i < THREAD_MAX_SIG_NUM; i++)
        {
            new_thread->signal_handlers[i] = NULL;
            new_thread->signal_num[i] = 0;
        }

        thread_enqueue(idle_queue, new_thread);

        log_puts("[thread_create] new_thread: 0x", THREAD_LOG_ON);
        log_puth((unsigned long)new_thread, THREAD_LOG_ON);
        log_puts("\n", THREAD_LOG_ON);
    }

    return new_thread;
}

void thread_exec (void (*prog)()) {

    thread_t *new_thread = NULL;
    pid_t pid;
    unsigned long current_el;

    if (idle_queue == NULL)
    {
        _thread_init();
    }

    pid = _get_new_pid();

    if (pid != -1)
    {
        new_thread = kmalloc(sizeof(thread_t));

        new_thread->pid          = pid;
        new_thread->status       = THREAD_IDLE;
        new_thread->code_addr    = (unsigned long)prog;
        new_thread->kernel_stack = (unsigned long)alloc_pages(4);
        new_thread->user_stack   = (unsigned long)alloc_pages(4);

        new_thread->context.lr = (unsigned long)prog;
        new_thread->context.fp = (unsigned long)new_thread->user_stack + THREAD_STACK_SIZE;
        new_thread->context.sp = (unsigned long)new_thread->user_stack + THREAD_STACK_SIZE;
        new_thread->next       = NULL;

        for (int i = 0; i < THREAD_MAX_SIG_NUM; i++)
        {
            new_thread->signal_handlers[i] = NULL;
            new_thread->signal_num[i] = 0;
        }

        unsigned long tmp;
        asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
        tmp |= 1;
        asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
        
        add_timer(_thread_timer_task, NULL, 5);

        // Get current EL
        asm volatile ("mrs %0, CurrentEL" : "=r" (current_el));
        current_el = current_el >> 2;

        // Print prompt
        uart_puts("Current EL: 0x");
        uart_puth(current_el);
        uart_puts("\n");
        uart_puts("User program at: 0x");
        uart_puth((unsigned long) prog);
        uart_puts("\n");
        uart_puts("User program stack top: 0x");
        uart_puth((unsigned long) new_thread->context.sp);
        uart_puts("\n");
        uart_puts("-----------------Entering user program-----------------\n");

        /* Enable RX interrupt */
        set_aux_int(false);
        set_uart_rx_int(false);
        set_uart_tx_int(false);

        asm volatile ("msr tpidr_el1, %0" : : "r"(new_thread));
        from_EL1_to_EL0((unsigned long)prog, (unsigned long)new_thread->user_stack + THREAD_STACK_SIZE, (unsigned long)new_thread->kernel_stack + THREAD_STACK_SIZE);
    }

    return;
}

void thread_schedule () {

    thread_t* curr_thread = NULL;
    thread_t* next_thread = NULL;

    curr_thread = get_current_thread();

    set_timer_interrupt(false);
    check_signal();
    set_timer_interrupt(true);
    
    while (1)
    {
        next_thread = thread_dequeue(idle_queue);

        if (next_thread == NULL)
        {
            break;
        }
        else if (next_thread->status == THREAD_IDLE)
        {
            break;
        }
        else if (next_thread->status == THREAD_EXIT)
        {
            pid_inuse[next_thread->pid] = false;
            free_page((void *)next_thread->kernel_stack);
            free_page((void *)next_thread->user_stack);
            kfree((void *)next_thread);
        }
    }

    if (next_thread != NULL)
    {

        log_puts("[thread_schedule] curr_thread: 0x", THREAD_LOG_ON);
        log_puth((unsigned long)curr_thread, THREAD_LOG_ON);
        log_puts(", next_thread: 0x", THREAD_LOG_ON);
        log_puth((unsigned long)next_thread, THREAD_LOG_ON);
        log_puts("\n", THREAD_LOG_ON);
        
        if (curr_thread->pid != 0)
        {
            thread_enqueue(idle_queue, curr_thread);
        }

        switch_to(&curr_thread->context, &next_thread->context, next_thread);

    }

    return;
}

void thread_kill (pid_t pid) {

    thread_t* curr_thread;

    set_interrupt(false);

    // Find pid thread
    curr_thread = idle_queue->front;

    while (curr_thread != NULL) {

        if (curr_thread->pid == pid)
        {
            log_puts("[Thread kill] Found thread pid: ", THREAD_LOG_ON);
            log_putu(curr_thread->pid, THREAD_LOG_ON);
            log_puts("\n", THREAD_LOG_ON);
            curr_thread->status = THREAD_EXIT;
            break;
        }

        log_puts("[Thread kill] curr_thread: 0x", THREAD_LOG_ON);
        log_puth((unsigned long)curr_thread, THREAD_LOG_ON);
        log_puts(", next: 0x", THREAD_LOG_ON);
        log_puth((unsigned long)curr_thread->next, THREAD_LOG_ON);
        log_puts("\n", THREAD_LOG_ON);
        curr_thread = curr_thread->next;
    }

    curr_thread = wait_queue->front;

    while (curr_thread != NULL) {

        if (curr_thread->pid == pid)
        {
            log_puts("[Thread kill] Found thread pid: ", THREAD_LOG_ON);
            log_putu(curr_thread->pid, THREAD_LOG_ON);
            log_puts("\n", THREAD_LOG_ON);
            curr_thread->status = THREAD_EXIT;
            break;
        }

        curr_thread = curr_thread->next;
    }

    set_interrupt(true);

    return;
}

pid_t thread_fork (trap_frame_t* trap_frame) {
    
    thread_t *parent_thread;
    thread_t *child_thread;
    thread_t *curr_thread;
    trap_frame_t *child_trap_frame;

    unsigned long k_offset, u_offset;

    set_interrupt(false);

    parent_thread = get_current_thread();
    child_thread  = thread_create((void *)parent_thread->code_addr);

    k_offset = (unsigned long)child_thread->kernel_stack - (unsigned long)parent_thread->kernel_stack;
    u_offset = (unsigned long)child_thread->user_stack - (unsigned long)parent_thread->user_stack;
    child_trap_frame = (trap_frame_t *)((unsigned long)trap_frame + k_offset);

    store_context(&child_thread->context);
    curr_thread = get_current_thread();

    if (curr_thread->pid == parent_thread->pid)
    {
        trap_frame->regs[0] = child_thread->pid;

        for (int i = 0; i < THREAD_STACK_SIZE; i++)
        {
            ((char *) child_thread->kernel_stack)[i] = ((char *) parent_thread->kernel_stack)[i];
            ((char *) child_thread->user_stack)[i] = ((char *) parent_thread->user_stack)[i];
        }

        for (int i = 0; i < THREAD_MAX_SIG_NUM; i++)
        {
            child_thread->signal_handlers[i] = parent_thread->signal_handlers[i];
            child_thread->signal_num[i] = parent_thread->signal_num[i];
        }

        child_thread->context.sp += k_offset;
        child_thread->context.fp += k_offset;
        child_trap_frame->sp_el0 += u_offset;
        child_trap_frame->regs[0] = 0;

        return child_thread->pid;
    }

    set_interrupt(true);

    return 0;
}

void thread_exit () {

    thread_t* curr_thread;

    curr_thread = get_current_thread();
    curr_thread->status = THREAD_EXIT;

    thread_schedule();

    return;
}

void _default_signal_handler () {

    thread_t* thread = get_current_thread();
    thread->status = THREAD_EXIT;
    
    return;
}

void check_signal () {

    void (*handler) ();
    void *signal_ustack;
    thread_t* thread = get_current_thread();

    unsigned long sp, spsr_el1;
    
    while (1) {
        
        store_context(&thread->signal_save_context);
        handler = _get_signal_handler(thread);

        if (handler == NULL)
        {
            break;
        }
        else
        {

            if (handler == _default_signal_handler)
            {
                // Run in kernel mode
                handler();
            }
            else
            {
                signal_ustack = alloc_pages(4);

                // Run in user mode
                asm volatile("msr     elr_el1, %0 \n\t"
                             "msr     sp_el0,  %1 \n\t"
                             "mov     lr, %2       \n\t"
                             "eret" :: "r" (handler), "r"(signal_ustack + THREAD_STACK_SIZE), "r" (sys_ret));

                free_page(signal_ustack);
            }
        }

    }

    return;
}

void *_get_signal_handler (thread_t* thread) {

    void *handler = NULL;

    for (int i = 0; i < THREAD_MAX_SIG_NUM; i++)
    {

        if (thread->signal_num[i] > 0)
        {

            if (thread->signal_handlers[i] == NULL)
            {
                // Default signal handler
                handler = _default_signal_handler;
            }
            else
            {
                handler = thread->signal_handlers[i];
            }

            thread->signal_num[i] -= 1;

            break;
        }

    }

    return handler;
}

void thread_signal_register (int signal, void (*handler)()) {
    
    thread_t* curr_thread;

    curr_thread = get_current_thread();

    if (signal > THREAD_MAX_SIG_NUM)
    {
        uart_puts("[thread_signal_register] Signal register error.\n");
    }

    curr_thread->signal_handlers[signal] = handler;

    log_puts("[thread_signal_register] signal: ", THREAD_LOG_ON);
    log_putu(signal, THREAD_LOG_ON);
    log_puts(", handler: 0x", THREAD_LOG_ON);
    log_puth(handler, THREAD_LOG_ON);
    log_puts("\n", THREAD_LOG_ON);

    return;
}

void thread_signal_kill (int pid, int signal) {

    thread_t* thread_ptr;

    thread_ptr = idle_queue->front;

    while (thread_ptr != NULL)
    {
        if (thread_ptr->pid == pid)
        {

            log_puts("[thread_signal_kill] signal: ", THREAD_LOG_ON);
            log_putu(signal, THREAD_LOG_ON);
            log_puts(", pid: ", THREAD_LOG_ON);
            log_putu(pid, THREAD_LOG_ON);
            log_puts("\n", THREAD_LOG_ON);

            thread_ptr->signal_num[signal] += 1;
            
            break;
        }

        thread_ptr = thread_ptr->next;
    }

    return;
}

void thread_signal_return () {

    thread_t* curr_thread;

    curr_thread = get_current_thread();

    load_context(&curr_thread->signal_save_context);

}

// Test
void idle_thread () {

    while (1)
    {
        log_puts("idle thread.\n", THREAD_LOG_ON);
        thread_schedule();
    }

}

void test_thread () {

    for (int i = 0; i < 5; i++)
    {
        uart_puts("HIHI I'm a thread ^u^, pid: ");
        uart_putu(get_current_thread()->pid);
        uart_puts(", iteration: ");
        uart_putu(i);
        uart_puts("\n");
        thread_schedule();
    }

    thread_exit();

}

void thread_test () {

    thread_create(idle_thread);

    for (int i = 0; i < 5; i++)
    {
        thread_create(test_thread);
    }

    thread_schedule();

}

void fork_test () {

    uart_puts("\nFork Test, pid ");
    uart_putu(sys_get_pid());
    uart_puts("\n");

    int cnt = 1;
    int ret = 0;

    ret = sys_fork();

    if (ret == 0) { // child
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));

        uart_puts("first child pid: ");
        uart_putu(sys_get_pid());
        uart_puts(", cnt: ");
        uart_putu(cnt);
        uart_puts(", ptr: 0x");
        uart_puth(&cnt);
        uart_puts(", sp : 0x");
        uart_puth(cur_sp);
        uart_puts("\n");

        ++cnt;

        if ((ret = sys_fork()) != 0){
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            
            uart_puts("first child pid: ");
            uart_putu(sys_get_pid());
            uart_puts(", cnt: ");
            uart_putu(cnt);
            uart_puts(", ptr: 0x");
            uart_puth(&cnt);
            uart_puts(", sp : 0x");
            uart_puth(cur_sp);
            uart_puts("\n");
        }
        else{
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));

                uart_puts("second child pid: ");
                uart_putu(sys_get_pid());
                uart_puts(", cnt: ");
                uart_putu(cnt);
                uart_puts(", ptr: 0x");
                uart_puth(&cnt);
                uart_puts(", sp : 0x");
                uart_puth(cur_sp);
                uart_puts("\n");

                ++cnt;
            }
        }

        sys_exit();
    }
    else {
        uart_puts("parent here, pid ");
        uart_putu(sys_get_pid());
        uart_puts(", child ");
        uart_putu(ret);
        uart_puts("\n");
    }
}