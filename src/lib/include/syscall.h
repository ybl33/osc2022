#ifndef __SYSCALL_H___
#define __SYSCALL_H___
#include "thread.h"
#include "uart.h"
#include "stddef.h"
#include "cpio.h"
#include "buddy.h"
#include "exception.h"

typedef struct trap_frame trap_frame_t;

int get_pid ();
size_t uart_read (char buf[], size_t size);
size_t uart_write (const char buf[], size_t size);
int exec (trap_frame_t *trap_frame, char* name, char **argv);
void exit();
void kill (int pid);
#endif