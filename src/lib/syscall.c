#include "syscall.h"

extern unsigned long CPIO_BASE;

int get_pid () {

    int pid = thread_get_pid();

    return pid;
}

size_t uart_read (char buf[], size_t size) {

    for (size_t i = 0; i < size; i++)
    {
        buf[i] = uart_get();
    }

    return size;
}

size_t uart_write (const char buf[], size_t size) {

    for (size_t i = 0; i < size; i++)
    {
        uart_put(buf[i]);
    }

    return size;
}

int exec (trap_frame_t *trap_frame, char* name, char **argv) {

    thread_t *curr_thread = get_current_thread();
    unsigned long prog_addr;
    unsigned int prog_size;

    cpio_load(name, &prog_addr, &prog_size);

    if (prog_size == 0)
    {
        uart_puts("User program not found!\n");
        return 1;
    }

    for (int i = 0; i < prog_size; i += PAGE_TABLE_SIZE)
    {
        unsigned long va = USER_PROG_VA + i;
        unsigned long pa = va_to_pa(prog_addr + i);

        alloc_page_table(curr_thread->pgd, va, pa, PD_USER_ATTR);
    }

    trap_frame->sp_el0  = (unsigned long)(USER_STACK_VA + THREAD_STACK_SIZE);
    trap_frame->elr_el1 = (unsigned long)USER_PROG_VA;

    return 0;
}

void exit() {

    thread_exit();

    return;
}

void kill (int pid) {

    thread_kill(pid);

    return;
}