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

    cpio_header_t *header = (cpio_header_t *)CPIO_BASE;
    thread_t *curr_thread = get_current_thread();
    void (*prog)();

    prog = cpio_load(header, name);

    curr_thread->code_addr = (unsigned long)prog;

    trap_frame->sp_el0  = (unsigned long)(curr_thread + THREAD_STACK_SIZE);
    trap_frame->elr_el1 = (unsigned long)prog;

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