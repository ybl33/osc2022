#include "uartfs.h"

int uartfs_mount(struct filesystem *fs, struct mount *mount) {

    struct vnode *mount_node;
    struct uartfs_internal *internal;
    char *name;

    internal       = kmalloc(sizeof(struct uartfs_internal));
    internal->node = kmalloc(sizeof(struct vnode));

    mount_node = mount->root;

    mount_node->v_ops->getname(mount_node, &name);

    internal->name          = name;
    internal->node->mount    = mount_node->mount;
    internal->node->v_ops    = mount_node->v_ops;
    internal->node->f_ops    = mount_node->f_ops;
    internal->node->parent   = mount_node->parent;
    internal->node->internal = mount_node->internal;

    mount_node->mount    = mount;
    mount_node->v_ops    = &uartfs_v_ops;
    mount_node->f_ops    = &uartfs_f_ops;
    mount_node->internal = internal;

    return 0;
}


int uartfs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name) {

    return -1;
}

int uartfs_create(struct vnode *dir_node, struct vnode **target, const char *component_name) {

    return -1;
}

int uartfs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name) {

    return -1;
}

int uartfs_getname(struct vnode *dir_node, const char **name) {

    struct uartfs_internal *internal;

    internal = dir_node->internal;

    *name = internal->name;

    return 0;
}

int uartfs_write(struct file *file, const void *buf, size_t len) {

    for (int i = 0; i < len; i++) 
    {
        uart_put(((char *)buf)[i]);
    }
    
    return len;
}

int uartfs_read(struct file *file, void *buf, size_t len) {

    for (int i = 0; i < len; i++) 
    {
        ((char *)buf)[i] = uart_get();
    }
    
    return len;
}

int uartfs_open(struct vnode *file_node, struct file *target) {

    target->vnode = file_node;
    target->f_pos = 0;
    target->f_ops = file_node->f_ops;

    return 0;
}

int uartfs_close(struct file *file) {

    file->vnode = NULL;
    file->f_pos = 0;
    file->f_ops = NULL;

    return 0;
}

long uartfs_lseek64(struct file *file, long offset, int whence) {

    return -1;
}

int uartfs_ioctl(struct file *file, unsigned long request, void *arg) {

    return -1;
}

struct filesystem uartfs = {
    .name = "uartfs",
    .mount = uartfs_mount
};

struct vnode_operations uartfs_v_ops = {
    .lookup  = uartfs_lookup,
    .create  = uartfs_create,
    .mkdir   = uartfs_mkdir,
    .getname = uartfs_getname,
};

 struct file_operations uartfs_f_ops = {
    .write   = uartfs_write,
    .read    = uartfs_read,
    .open    = uartfs_open,
    .close   = uartfs_close,
    .lseek64 = uartfs_lseek64,
    .ioctl   = uartfs_ioctl
};

struct filesystem *uartfs_init () {

    return &uartfs;
}