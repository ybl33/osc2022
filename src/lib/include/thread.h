#ifndef __THREAD_H___
#define __THREAD_H___
#include "timer.h"
#include "slab.h"
#include "stddef.h"
#include "cpio.h"
#include "log.h"
#include "mmu.h"

#define THREAD_MAX_NUM               (32)
#define THREAD_STACK_SIZE            (4096)
#define THREAD_IDLE                  (0)
#define THREAD_RUNNING               (1)
#define THREAD_WAIT                  (2)
#define THREAD_EXIT                  (3)
#define THREAD_MAX_SIG_NUM           (16)

#define THREAD_LOG_ON                (0)

#define USER_PROG_VA                 (0x0)
#define USER_STACK_VA                (0xFFFFFFFFD000)
#define KERNEL_STACK_VA              (0xFFFFFFFFE000)

struct thread;

typedef int pid_t;

typedef struct cpio_header cpio_header_t;

typedef struct thread_context {

    unsigned long x19;
    unsigned long x20;
    unsigned long x21;
    unsigned long x22;
    unsigned long x23;
    unsigned long x24;
    unsigned long x25;
    unsigned long x26;
    unsigned long x27;
    unsigned long x28;
    unsigned long fp;
    unsigned long lr;
    unsigned long sp;

} thread_context_t;

typedef struct thread {

    thread_context_t context;
    pid_t pid;
    unsigned long code_addr;
    unsigned long kernel_stack;
    unsigned long user_stack;
    unsigned int status;
    unsigned int code_size;

    // For POSIX Signal
    thread_context_t signal_save_context;

    void (*signal_handlers[THREAD_MAX_SIG_NUM]) ();
    unsigned int signal_num[THREAD_MAX_SIG_NUM];

    unsigned long *pgd;

    struct thread *next;

} thread_t;

typedef struct thread_queue {

    thread_t* front;
    thread_t* tail;

} thread_queue_t;

typedef struct trap_frame trap_frame_t;

thread_t* get_current_thread();
pid_t thread_get_pid();
thread_t* thread_create( void (*func) () );
void thread_exec(char *file_name);
void thread_schedule();
void thread_kill(pid_t pid);
pid_t thread_fork(trap_frame_t* trap_frame);
void thread_exit();
void thread_test();
void idle_thread();

void _default_signal_handler();
void check_signal();
void *_get_signal_handler(thread_t* thread);
void thread_signal_register(int signal, void (*handler)());
void thread_signal_kill(int pid, int signal);
void thread_signal_return();

void thread_test();
void fork_test();

#endif