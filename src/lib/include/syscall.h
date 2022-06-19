#ifndef __SYSCALL_H___
#define __SYSCALL_H___
#include "thread.h"
#include "uart.h"
#include "stddef.h"
#include "cpio.h"
#include "buddy.h"
#include "exception.h"
#include "vfs.h"
#include "log.h"

#define SYSCALL_DEBUG (0)

typedef struct trap_frame trap_frame_t;

int get_pid ();
size_t uart_read (char buf[], size_t size);
size_t uart_write (const char buf[], size_t size);
int exec (trap_frame_t *trap_frame, char* name, char **argv);
void exit();
void kill (int pid);
// VFS syscalls
int open(const char *pathname, int flags);
int close(int fd);
long write(int fd, const void *buf, unsigned long count);
long read(int fd, void *buf, unsigned long count);
int mkdir(const char *pathname, unsigned mode);
int mount(const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data);
int chdir(const char *path);
long lseek64(int fd, unsigned long offset, int whence);
int ioctl(int fd, unsigned long request, void *arg);

#endif