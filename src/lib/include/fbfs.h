#ifndef __FBFS_H__
#define __FBFS_H__

#include "stddef.h"
#include "string.h"
#include "slab.h"
#include "vfs.h"
#include "mbox.h"
#include "utils.h"

struct fb_info {
    unsigned int width;
    unsigned int height;
    unsigned int pitch;
    unsigned int isrgb;
};

struct fbfs_internal {
    const char *name;
    unsigned char *lfb;
    unsigned int lfbsize;
    struct vnode *node;
};

extern struct filesystem fbfs;
extern struct vnode_operations fbfs_v_ops;
extern struct file_operations fbfs_f_ops;
struct filesystem* fbfs_init();

#endif