#ifndef __TMPFS_H__
#define __TMPFS_H__

#include "string.h"
#include "slab.h"
#include "vfs.h"
#include "uartfs.h"
#include "log.h"

#define TMPFS_FILE_MAX_SIZE  (4096)
#define TMPFS_DIR_MAX_SIZE   (16)
#define TMPFS_IS_UNDEFIENDED (0)
#define TMPFS_IS_FILE        (1)
#define TMPFS_IS_DIR         (2)
#define TMPFS_DEBUG          (0)

struct tmpfs_file {
    char* data;
    unsigned int size;
    unsigned int capacity;
};

struct tmpfs_dir {
    int size;
    struct vnode *nodes[TMPFS_DIR_MAX_SIZE];
};

struct tmpfs_internal {
    char name[256];
    int type;
    struct tmpfs_file *file;
    struct tmpfs_dir  *dir;
    struct vnode *node;
};

extern struct filesystem tmpfs;
extern struct vnode_operations tmpfs_v_ops;
extern struct file_operations tmpfs_f_ops;

struct filesystem* tmpfs_init(struct vnode **root_node);

#endif