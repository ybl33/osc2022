#ifndef __VFS__H__
#define __VFS__H__

#include "stdarg.h"
#include "stddef.h"
#include "string.h"
#include "slab.h"
#include "list.h"
#include "thread.h"
#include "tmpfs.h"
#include "fbfs.h"
#include "cpiofs.h"
#include "log.h"

#define O_CREAT   (00000100)
#define SEEK_SET  (0)
#define SEEK_CUR  (1)
#define SEEK_END  (2)
#define VFS_DEBUG (0)

struct vnode {
    struct mount* mount;
    struct vnode_operations* v_ops;
    struct file_operations* f_ops;
    struct vnode *parent;
    void* internal;
};

// file handle
struct file {
    struct vnode* vnode;
    size_t f_pos;  // RW position of this file handle
    struct file_operations* f_ops;
    int flags;
};

struct mount {
    struct vnode* root;
    struct filesystem* fs;
};

struct filesystem {
    list_t list_node;
    const char* name;
    int (*mount)(struct filesystem* fs, struct mount* mount);
};

struct file_operations {
    int (*write)(struct file* file, const void* buf, size_t len);
    int (*read)(struct file* file, void* buf, size_t len);
    int (*open)(struct vnode* file_node, struct file* target);
    int (*close)(struct file* file);
    long (*lseek64)(struct file *file, long offset, int whence);
    int (*ioctl)(struct file *file, unsigned long request, void *arg);
};

struct vnode_operations {
    int (*lookup)(struct vnode* dir_node, struct vnode** target, const char* component_name);
    int (*create)(struct vnode* dir_node, struct vnode** target, const char* component_name);
    int (*mkdir)(struct vnode* dir_node, struct vnode** target, const char* component_name);
    int (*getname)(struct vnode *dir_node, const char **name);
    int (*getsize)(struct vnode *dir_node);
};

extern struct mount *rootfs;

struct filesystem* get_filesystem(char *fs_name);
int register_filesystem(struct filesystem* fs);
int vfs_open(const char* pathname, int flags, struct file* target);
int vfs_close(struct file* file);
int vfs_write(struct file* file, const void* buf, size_t len);
int vfs_read(struct file* file, void* buf, size_t len);
int vfs_mkdir(const char* pathname);
int vfs_mount(const char* target, const char* filesystem);
int vfs_lookup(const char* pathname, struct vnode** target);
long vfs_lseek64(struct file *file, long offset, int whence);
int vfs_ioctl(struct file *file, unsigned long request, void *arg);
void vfs_init ();
void vfs_dump();
#endif