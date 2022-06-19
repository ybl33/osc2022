#include "vfs.h"

list_t* fs_list = NULL;
struct mount *rootfs;

struct filesystem* get_filesystem(char *fs_name) {

    struct filesystem* curr_fs = fs_list;

    if (curr_fs == NULL)
    {
        return NULL;
    }

    do {

        if (!strcmp(curr_fs->name, fs_name))
        {
            break;
        }

        curr_fs = (struct filesystem*)(curr_fs->list_node.next);

    } while (curr_fs != fs_list);

    return curr_fs;
}

struct vnode* get_dir_vnode (struct vnode *curr_dir_vnode, const char **pathname) {

    struct vnode *dir_vnode;
    const char *head, *tail;
    char lookup_name[256];

    head = tail = *pathname;

    log_puts("[get_dir_vnode] pathname: ", VFS_DEBUG);
    log_puts(*pathname, VFS_DEBUG);
    log_puts("\n", VFS_DEBUG);
    
    if (head[0] == '/')
    {
        // Start from root
        dir_vnode = rootfs->root;
    }
    else
    {
        // Start from current directory
        dir_vnode = curr_dir_vnode;
    }

    while (1)
    {

        if (!strncmp("./", head, 2))
        {
            head += 2;
            tail = head;
        }
        else if (!strncmp("../", head, 3))
        {
            head += 3;
            tail = head;
            dir_vnode = curr_dir_vnode->parent;
        }
        
        while (*tail != '\0' && *tail != '/')
        {
            tail++;
        }

        if (*tail == '/')
        {
            if (head == tail) 
            {
                tail++;
                head = tail;
                continue;
            }

            memcpy(lookup_name, head, tail - head);
            lookup_name[tail - head] = '\0';

            log_puts("[get_dir_vnode] head: ", VFS_DEBUG);
            log_puts(head, VFS_DEBUG);
            log_puts(", tail: ", VFS_DEBUG);
            log_puts(tail, VFS_DEBUG);
            log_puts(", lookup_name: ", VFS_DEBUG);
            log_puts(lookup_name, VFS_DEBUG);
            log_puts("\n", VFS_DEBUG);

            if (dir_vnode->v_ops->lookup(dir_vnode, &dir_vnode, lookup_name))
            {
                log_puts("not found.\n", VFS_DEBUG);
                return 0;
            }

            tail++;
            head = tail;
        }
        else
        {
            break;
        }
    }

    if (*head != '\0')
    {
        *pathname = head;
    }
    else
    {
        *pathname = NULL;
    }

    char *dirname;
    dir_vnode->v_ops->getname(dir_vnode, &dirname);
    log_puts("[get_dir_vnode] pathname: ", VFS_DEBUG);
    log_puts(*pathname, VFS_DEBUG);
    log_puts(", dir_vnode: 0x", VFS_DEBUG);
    log_puth(dir_vnode, VFS_DEBUG);
    log_puts(", dirname: ", VFS_DEBUG);
    log_puts(dirname, VFS_DEBUG);
    log_puts("\n", VFS_DEBUG);


    return dir_vnode;
}

int register_filesystem(struct filesystem* fs) {

    if (fs_list == NULL) 
    {
        fs_list = &(fs->list_node);
        list_init(fs_list);
    }
    else
    {
        list_push(fs_list, &(fs->list_node));
    }

    vfs_dump();

    return 0;
}

int vfs_open(const char* pathname, int flags, struct file* target) {

    struct vnode *dir_vnode;
    struct vnode *file_vnode;
    thread_t *curr_thread = get_current_thread();
    int ret = -1;

    log_puts("[vfs_open] pathname: ", VFS_DEBUG);
    log_puts(pathname, VFS_DEBUG);
    log_puts("\n", VFS_DEBUG);

    if (curr_thread == NULL)
    {
        dir_vnode = get_dir_vnode(rootfs->root, &pathname);
    }
    else
    {
        dir_vnode = get_dir_vnode(curr_thread->working_dir, &pathname);
    }

    char *name;
    dir_vnode->v_ops->getname(dir_vnode, &name);
    log_puts("[vfs_open] pathname: ", VFS_DEBUG);
    log_puts(pathname, VFS_DEBUG);
    log_puts(", dir_vnode name: ", VFS_DEBUG);
    log_puts(name, VFS_DEBUG);
    log_puts("\n", VFS_DEBUG);

    if (dir_vnode == NULL || pathname == NULL)
    {
        return -1;
    }

    ret = dir_vnode->v_ops->lookup(dir_vnode, &file_vnode, pathname);

    if (flags & O_CREAT)
    {

        if (ret == 0)
        {
            log_puts("[vfs_open] already exist.\n", VFS_DEBUG);
            // Already exist
            return -1;
        }

        ret = dir_vnode->v_ops->create(dir_vnode, &file_vnode, pathname);
    }

    if (ret < 0 || (file_vnode == NULL))
    {
        log_puts("[vfs_open] create failed.\n", VFS_DEBUG);
        return -1;
    }

    ret = dir_vnode->f_ops->open(file_vnode, target);

    if (ret < 0)
    {
        log_puts("[vfs_open] open failed.\n", VFS_DEBUG);
        return -1;
    }

    return 0;
}

int vfs_close(struct file* file) {
    
    int ret = file->f_ops->close(file);

    return ret;
}

int vfs_write(struct file* file, const void* buf, size_t len) {

    int ret = file->f_ops->write(file, buf, len);

    return ret;
}

int vfs_read(struct file* file, void* buf, size_t len) {

    int ret = file->f_ops->read(file, buf, len);

    return ret;
}

int vfs_mkdir(const char* pathname) {

    struct vnode *dir_vnode;
    struct vnode *new_dir_vnode;
    thread_t *curr_thread = get_current_thread();
    int ret;

    log_puts("[vfs_mkdir] pathname: ", VFS_DEBUG);
    log_puts(pathname, VFS_DEBUG);
    log_puts("\n", VFS_DEBUG);

    if (curr_thread == NULL)
    {
        dir_vnode = get_dir_vnode(rootfs->root, &pathname);
    }
    else
    {
        dir_vnode = get_dir_vnode(curr_thread->working_dir, &pathname);
    }

    if (dir_vnode == NULL || pathname == NULL)
    {
        return -1;
    }

    ret = dir_vnode->v_ops->mkdir(dir_vnode, &new_dir_vnode, pathname);


    // Debug
    new_dir_vnode->v_ops->getname(new_dir_vnode, &pathname);
    log_puts("[vfs_mkdir] name: ", VFS_DEBUG);
    log_puts(pathname, VFS_DEBUG);
    log_puts("\n", VFS_DEBUG);

    return ret;
}

int vfs_mount(const char* target, const char* filesystem) {
    
    struct vnode *dir_vnode;
    struct filesystem* fs = get_filesystem(filesystem);
    thread_t *curr_thread = get_current_thread();
    int ret = -1;

    log_puts("[vfs_mount] fs->name: ", VFS_DEBUG);
    log_puts(fs->name, VFS_DEBUG);
    log_puts("\n", VFS_DEBUG);

    if (fs == NULL)
    {
        return -1;
    }

    if (curr_thread == NULL)
    {
        dir_vnode = get_dir_vnode(rootfs->root, &target);
    }
    else
    {
        dir_vnode = get_dir_vnode(curr_thread->working_dir, &target);
    }

    if (dir_vnode == NULL)
    {
        log_puts("[vfs_mount] failed.\n", VFS_DEBUG);
        return -1;
    }
    else
    {
        struct mount *new_mount = kmalloc(sizeof(struct mount));

        if (target) 
        {
            ret = dir_vnode->v_ops->lookup(dir_vnode, &dir_vnode, target);
            
            if (ret < 0) {
                return ret;
            }
        }

        new_mount->root = dir_vnode;
        new_mount->fs   = fs;

        ret = fs->mount(fs, new_mount);
    }

    return ret;
}

int vfs_lookup(const char* pathname, struct vnode** target) {
    
    struct vnode *dir_vnode;
    thread_t *curr_thread = get_current_thread();

    if (curr_thread == NULL)
    {
        dir_vnode = get_dir_vnode(rootfs->root, &pathname);
    }
    else
    {
        dir_vnode = get_dir_vnode(curr_thread->working_dir, &pathname);
    }

    if (dir_vnode == NULL)
    {
        return -1;
    }

    if (pathname == NULL)
    {
        // is dir
        *target = dir_vnode;

        return 0;
    }
    
    return dir_vnode->v_ops->lookup(dir_vnode, target, pathname);
}

long vfs_lseek64(struct file *file, long offset, int whence) {

    long ret = file->f_ops->lseek64(file, offset, whence);

    return ret;
}

int vfs_ioctl(struct file *file, unsigned long request, void *arg) {

    int ret = file->f_ops->ioctl(file, request, arg);

    return ret;
}

void vfs_init () {
    
    struct vnode *root_node;
    struct filesystem *tmpfs, *uartfs, *fbfs, *cpiofs;

    // init rootfs
    tmpfs  = tmpfs_init(&root_node);
    rootfs = kmalloc(sizeof(struct mount));
    rootfs->root = root_node;
    rootfs->fs   = tmpfs;

    root_node->mount  = rootfs; 
    root_node->parent = NULL;

    register_filesystem(tmpfs);

    // mount devices
    vfs_mkdir("/dev");
    // init uartfs
    uartfs = uartfs_init();
    register_filesystem(uartfs);
    vfs_mkdir("/dev/uart");
    vfs_mount("/dev/uart", "uartfs");

    // init fbfs
    fbfs = fbfs_init();
    register_filesystem(fbfs);
    vfs_mkdir("/dev/framebuffer");
    vfs_mount("/dev/framebuffer", "fbfs");

    // init cpiofs
    cpiofs = cpiofs_init();
    register_filesystem(cpiofs);
    vfs_mkdir("/initramfs");
    vfs_mount("/initramfs", "cpiofs");

    return;
}

void vfs_dump () {

    log_puts("[fs_list] ", VFS_DEBUG);

    list_t* curr = fs_list;

    do {
        log_puth(curr, VFS_DEBUG);
        log_puts("(", VFS_DEBUG);
        log_puts(((struct filesystem*)curr)->name, VFS_DEBUG);
        log_puts(")", VFS_DEBUG);
        log_puts(" -> ", VFS_DEBUG);
        curr = curr->next;
    } while (curr != fs_list);

    log_puts("\n", VFS_DEBUG);

}