#ifndef __CPIOFS_H__
#define __CPIOFS_H__

#include "cpio.h"
#include "string.h"
#include "slab.h"
#include "vfs.h"
#include "stddef.h"
#include "log.h"

#define CPIOFS_IS_UNDEFIENDED (0)
#define CPIOFS_IS_FILE        (1)
#define CPIOFS_IS_DIR         (2)
#define CPIOFS_DIR_MAX_SIZE   (3)
#define CPIOFS_DEBUG          (0)

struct cpiofs_file {
    char* data;
    unsigned long size;
};

struct cpiofs_dir {
    int size;
    struct vnode *nodes[CPIOFS_DIR_MAX_SIZE];
};

struct cpiofs_internal {
    char *name;
    int type;
    struct cpiofs_file file;
    struct cpiofs_dir  dir;
    struct vnode *node;
};

extern struct filesystem cpiofs;
extern struct vnode_operations cpiofs_v_ops;
extern struct file_operations cpiofs_f_ops;

struct filesystem* cpiofs_init();

#endif