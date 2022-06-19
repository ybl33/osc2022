#ifndef __UARTFS_H__
#define __UARTFS_H__

#include "string.h"
#include "slab.h"
#include "vfs.h"

struct uartfs_internal {
    const char *name;
    struct vnode *node;
};

extern struct filesystem uartfs;
extern struct vnode_operations uartfs_v_ops;
extern struct file_operations uartfs_f_ops;
struct filesystem* uartfs_init();

#endif