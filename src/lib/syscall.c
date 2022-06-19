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
    unsigned long prog_base;
    unsigned int prog_size;
    struct file prog;

    if (vfs_open(name, 0, &prog) < 0) 
    {
        uart_puts("User program: '");
        uart_puts(name);
        uart_puts("' not found!\n");
        return 1;
    }

    prog_size = prog.vnode->v_ops->getsize(prog.vnode);
    prog_base = alloc_pages((prog_size + BUDDY_PAGE_SIZE - 1) / BUDDY_PAGE_SIZE);
    vfs_read(&prog, prog_base, prog_size);
    
    for (int i = 0; i < prog_size; i += PAGE_TABLE_SIZE)
    {
        unsigned long va = USER_PROG_VA + i;
        unsigned long pa = va_to_pa(prog_base + i);

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

int open(const char *pathname, int flags) {

    int fd  = -1;
    int ret = 0;
    thread_t *curr_thread = get_current_thread();

    log_puts("[open] pathname: ", SYSCALL_DEBUG);
    log_puts(pathname, SYSCALL_DEBUG);
    log_puts(", flags: 0x", SYSCALL_DEBUG);
    log_puth(flags, SYSCALL_DEBUG);
    log_puts("\n", SYSCALL_DEBUG);

    for (int i = 0; i < THREAD_FD_TABLE_SIZE; i++)
    {
        if (curr_thread->fd_table[i].vnode == NULL)
        {
            fd = i;
            break;
        }
    }

    if (fd >= THREAD_FD_TABLE_SIZE)
    {
        return -1;
    }
    else
    {
        ret = vfs_open(pathname, flags, &curr_thread->fd_table[fd]);
    }

    if (ret < 0)
    {
        log_puts("[open] failed.\n", SYSCALL_DEBUG);
        curr_thread->fd_table[fd].vnode = NULL;
        fd = ret;
    }

    log_puts("[open] fd: ", SYSCALL_DEBUG);
    log_puth(fd, SYSCALL_DEBUG);
    log_puts(", curr_thread->fd_table[fd].vnode: 0x", SYSCALL_DEBUG);
    log_puth(curr_thread->fd_table[fd].vnode, SYSCALL_DEBUG);
    log_puts("\n", SYSCALL_DEBUG);
    
    return fd;
}

int close(int fd) {

    int ret = 0;
    thread_t *curr_thread = get_current_thread();

    if (fd < 0 || fd >= THREAD_FD_TABLE_SIZE)
    {
        return -1;
    }

    if (curr_thread->fd_table[fd].vnode == NULL)
    {
        return -1;
    }

    ret = vfs_close(&curr_thread->fd_table[fd]);

    if (ret < 0)
    {
        return ret;
    }

    return 0;
}

long write(int fd, const void *buf, unsigned long count) {

    int ret = 0;
    thread_t *curr_thread = get_current_thread();

    if (fd < 0 || fd >= THREAD_FD_TABLE_SIZE)
    {
        return -1;
    }

    if (curr_thread->fd_table[fd].vnode == NULL)
    {
        return -1;
    }

    ret = vfs_write(&curr_thread->fd_table[fd], buf, count);

    return ret;
}

long read(int fd, void *buf, unsigned long count) {

    int ret = 0;
    thread_t *curr_thread = get_current_thread();

    if (fd < 0 || fd >= THREAD_FD_TABLE_SIZE)
    {
        return -1;
    }

    if (curr_thread->fd_table[fd].vnode == NULL)
    {
        return -1;
    }

    ret = vfs_read(&curr_thread->fd_table[fd], buf, count);

    return ret;
}

int mkdir(const char *pathname, unsigned mode) {

    return vfs_mkdir(pathname);
}

int mount(const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data) {

    log_puts("[mount] target: ", SYSCALL_DEBUG);
    log_puts(target, SYSCALL_DEBUG);
    log_puts(", filesystem: ", SYSCALL_DEBUG);
    log_puts(filesystem, SYSCALL_DEBUG);
    log_puts("\n", SYSCALL_DEBUG);

    return vfs_mount(target, filesystem);
}

int chdir(const char *path) {

    thread_t *curr_thread = get_current_thread();
    struct vnode *target_dir;
    int ret;

    ret = vfs_lookup(path, &target_dir);

    if (ret < 0) {
        return ret;
    }

    curr_thread->working_dir = target_dir;

    char *curr_dir_name;
    target_dir->v_ops->getname(target_dir, &curr_dir_name);
    log_puts("[chdir] curr : ", SYSCALL_DEBUG);
    log_puts(curr_dir_name, SYSCALL_DEBUG);
    log_puts("\n", SYSCALL_DEBUG);

    return 0;
}

long lseek64(int fd, unsigned long offset, int whence) {

    thread_t *curr_thread = get_current_thread();
    long ret;

    if (fd < 0 || fd >= THREAD_FD_TABLE_SIZE)
    {
        return -1;
    }

    if (curr_thread->fd_table[fd].vnode == NULL)
    {
        return -1;
    }

    ret = vfs_lseek64(&curr_thread->fd_table[fd], offset, whence);

    return ret;
}

int ioctl(int fd, unsigned long request, void *arg) {

    thread_t *curr_thread = get_current_thread();
    int ret;

    if (fd < 0 || fd >= THREAD_FD_TABLE_SIZE)
    {
        return -1;
    }

    if (curr_thread->fd_table[fd].vnode == NULL)
    {
        return -1;
    }

    ret = vfs_ioctl(&curr_thread->fd_table[fd], request, arg);

    return ret;
}