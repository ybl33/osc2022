#include "tmpfs.h"

int tmpfs_mount(struct filesystem *fs, struct mount *mount) {
    
    struct vnode *new_node, *mount_node;
    struct tmpfs_dir *dir;
    struct tmpfs_internal *internal;
    char *name;

    new_node = kmalloc(sizeof(struct vnode));
    dir      = kmalloc(sizeof(struct tmpfs_dir));
    internal = kmalloc(sizeof(struct tmpfs_internal));

    mount_node = mount->root;

    new_node->mount    = mount_node->mount;
    new_node->v_ops    = mount_node->v_ops;
    new_node->f_ops    = mount_node->f_ops;
    new_node->internal = mount_node->internal;

    dir->size = 0;

    mount_node->v_ops->getname(mount_node, &name);

    strcpy(internal->name, name);

    internal->type = TMPFS_IS_DIR;
    internal->dir  = dir;
    internal->node = new_node;

    mount_node->mount = mount;
    mount_node->v_ops = &tmpfs_v_ops;
    mount_node->f_ops = &tmpfs_f_ops;
    mount_node->internal = internal;

    log_puts("[tmpfs_mount] dir: 0x", TMPFS_DEBUG);
    log_puth(dir, TMPFS_DEBUG);
    log_puts("\n", TMPFS_DEBUG);

    return 0;
}

int tmpfs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name) {
    
    struct vnode *node;
    struct tmpfs_dir *dir;
    struct tmpfs_internal *internal;
    char *name;
    int idx;

    internal = dir_node->internal;

    if (internal->type == TMPFS_IS_DIR)
    {
        dir = internal->dir;

        log_puts("[tmpfs_lookup] dir_node: 0x", TMPFS_DEBUG);
        log_puth(dir_node, TMPFS_DEBUG);
        log_puts(", dir->size: ", TMPFS_DEBUG);
        log_putu(dir->size, TMPFS_DEBUG);
        log_puts("\n", TMPFS_DEBUG);

        for(idx = 0; idx < dir->size; idx++)
        {
            node = dir->nodes[idx];
            node->v_ops->getname(node, &name);

            log_puts("[tmpfs_lookup] name: ", TMPFS_DEBUG);
            log_puts(name, TMPFS_DEBUG);
            log_puts("\n", TMPFS_DEBUG);

            if (!strcmp(name, component_name))
            {
                *target = node;
                return 0;
            }
        }

    }

    return -1;
}

int tmpfs_create(struct vnode *dir_node, struct vnode **target, const char *component_name) {

    struct tmpfs_internal *new_internal, *curr_internal;
    struct tmpfs_file *file;
    struct tmpfs_dir  *dir;
    struct vnode *node;
    int ret = -1;

    curr_internal = dir_node->internal;

    if (curr_internal->type == TMPFS_IS_DIR)
    {
        dir = curr_internal->dir;

        if (dir->size < TMPFS_DIR_MAX_SIZE)
        {
            ret = tmpfs_lookup(dir_node, &node, component_name);

            if (ret != 0) // Create a new node
            {
                node = kmalloc(sizeof(struct vnode));
                file = kmalloc(sizeof(struct tmpfs_file));
                new_internal = kmalloc(sizeof(struct tmpfs_internal));

                // setup file
                file->data = alloc_pages(1);
                file->size = 0;
                file->capacity = TMPFS_FILE_MAX_SIZE;

                // setup internal
                strcpy(new_internal->name, component_name);
                new_internal->type = TMPFS_IS_FILE;
                new_internal->file = file;
                new_internal->node = NULL;

                // setup node
                node->mount    = dir_node->mount;
                node->v_ops    = &tmpfs_v_ops;
                node->f_ops    = &tmpfs_f_ops;
                node->parent   = dir_node;
                node->internal = new_internal;

                // put into dir
                dir->nodes[dir->size] = node;
                dir->size++;

                *target = node;

                ret = 0;
            }
        }

    }

    return ret;
}

int tmpfs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name) {

    struct tmpfs_internal *new_internal, *curr_internal;
    struct tmpfs_dir *new_dir, *dir;
    struct vnode *node;
    int ret = -1;

    curr_internal = dir_node->internal;

    if (curr_internal->type == TMPFS_IS_DIR)
    {
        dir = curr_internal->dir;

        if (dir->size < TMPFS_DIR_MAX_SIZE)
        {
            ret = tmpfs_lookup(dir_node, &node, component_name);

            if (ret != 0) // Create a new node
            {
                node    = kmalloc(sizeof(struct vnode));
                new_dir = kmalloc(sizeof(struct tmpfs_dir));
                new_internal = kmalloc(sizeof(struct tmpfs_internal));

                // setup dir
                new_dir->size = 0;

                // setup internal
                strcpy(new_internal->name, component_name);
                new_internal->type = TMPFS_IS_DIR;
                new_internal->dir  = new_dir;
                new_internal->node = NULL;

                // setup node
                node->mount    = dir_node->mount;
                node->v_ops    = &tmpfs_v_ops;
                node->f_ops    = &tmpfs_f_ops;
                node->parent   = dir_node;
                node->internal = new_internal;

                // put into dir
                dir->nodes[dir->size] = node;
                dir->size++;

                *target = node;

                ret = 0;
            }
        }

    }

    return ret;
}

int tmpfs_write(struct file *file, const void *buf, size_t len) {

    struct tmpfs_file *f_internal;
    struct tmpfs_internal *internal;

    internal = file->vnode->internal;

    if (internal->type == TMPFS_IS_FILE)
    {
        f_internal = internal->file;

        log_puts("f_internal->size: ", TMPFS_DEBUG);
        log_putu(f_internal->size, TMPFS_DEBUG);
        log_puts(", file->f_pos: ", TMPFS_DEBUG);
        log_putu(file->f_pos, TMPFS_DEBUG);
        log_puts("\n", TMPFS_DEBUG);
        log_puts("len: ", TMPFS_DEBUG);
        log_putu(len, TMPFS_DEBUG);
        log_puts("\n", TMPFS_DEBUG);

        if (len > (f_internal->capacity - file->f_pos))
        {
            // Not enough space, write partial
            len = (f_internal->capacity - file->f_pos);
        }

        log_puts("len: ", TMPFS_DEBUG);
        log_putu(len, TMPFS_DEBUG);
        log_puts("\n", TMPFS_DEBUG);


        if (len != 0)
        {
            memcpy(&f_internal->data[file->f_pos], buf, len);
            file->f_pos += len;

            if (file->f_pos > f_internal->size) {
                f_internal->size = file->f_pos;
            }
        }

        // Debug
        char *filename;
        file->vnode->v_ops->getname(file->vnode, &filename);
        log_puts("[tmpfs_write] filename: ", TMPFS_DEBUG);
        log_puts(filename, TMPFS_DEBUG);
        log_puts(", file->vnode: 0x", TMPFS_DEBUG);
        log_puth(file->vnode, TMPFS_DEBUG);
        log_puts(", file->vnode->internal: 0x", TMPFS_DEBUG);
        log_puth(file->vnode->internal, TMPFS_DEBUG);
        log_puts(", f_internal->data: 0x", TMPFS_DEBUG);
        log_puth(f_internal->data, TMPFS_DEBUG);
        log_puts(", buf: ", TMPFS_DEBUG);
        for (int i = 0; i < len; i++) log_put(((char*)buf)[i], TMPFS_DEBUG);
        log_puts("\n", TMPFS_DEBUG);


        return len;
    }

    return -1;
}

int tmpfs_read(struct file *file, void *buf, size_t len) {

    struct tmpfs_file *f_internal;
    struct tmpfs_internal *internal;

    internal = file->vnode->internal;

    if (internal->type == TMPFS_IS_FILE)
    {

        f_internal = internal->file;

        log_puts("f_internal->size: ", TMPFS_DEBUG);
        log_putu(f_internal->size, TMPFS_DEBUG);
        log_puts(", file->f_pos: ", TMPFS_DEBUG);
        log_putu(file->f_pos, TMPFS_DEBUG);
        log_puts("\n", TMPFS_DEBUG);
        log_puts("len: ", TMPFS_DEBUG);
        log_putu(len, TMPFS_DEBUG);
        log_puts("\n", TMPFS_DEBUG);

        if (len > (f_internal->size - file->f_pos)) 
        {
            len = (f_internal->size - file->f_pos);
        }
        log_puts("len: ", TMPFS_DEBUG);
        log_putu(len, TMPFS_DEBUG);
        log_puts("\n", TMPFS_DEBUG);

        if (len != 0)
        {
            memcpy(buf, &f_internal->data[file->f_pos], len);
            file->f_pos += len;

        }

        // Debug
        char *filename;
        file->vnode->v_ops->getname(file->vnode, &filename);
        log_puts("[tmpfs_read] filename: ", TMPFS_DEBUG);
        log_puts(filename, TMPFS_DEBUG);
        log_puts(", file->vnode: 0x", TMPFS_DEBUG);
        log_puth(file->vnode, TMPFS_DEBUG);
        log_puts(", file->vnode->internal: 0x", TMPFS_DEBUG);
        log_puth(file->vnode->internal, TMPFS_DEBUG);
        log_puts(", f_internal->data: 0x", TMPFS_DEBUG);
        log_puth(f_internal->data, TMPFS_DEBUG);
        log_puts(", buf: ", TMPFS_DEBUG);
        for (int i = 0; i < len; i++) log_put(((char*)buf)[i], TMPFS_DEBUG);
        log_puts("\n", TMPFS_DEBUG);

        return len;
    }

    return -1;
}

int tmpfs_open(struct vnode *file_node, struct file *target) {

    target->vnode = file_node;
    target->f_pos = 0;
    target->f_ops = file_node->f_ops;

    return 0;
}

int tmpfs_close(struct file *file) {

    file->vnode = NULL;
    file->f_pos = 0;
    file->f_ops = NULL;

    return 0;
}

long tmpfs_lseek64(struct file *file, long offset, int whence) {

    return -1;
}

int tmpfs_ioctl(struct file *file, unsigned long request, void *arg) {

    return -1;
}

int tmpfs_getname(struct vnode *dir_node, const char **name) {

    struct tmpfs_internal *internal;

    internal = dir_node->internal;

    *name = internal->name;

    return 0;
}

struct filesystem tmpfs  = {
    .name = "tmpfs",
    .mount = tmpfs_mount,
};

struct vnode_operations tmpfs_v_ops  = {
    .lookup = tmpfs_lookup,
    .create = tmpfs_create,
    .mkdir = tmpfs_mkdir,
    .getname = tmpfs_getname
};

struct file_operations tmpfs_f_ops  = {
    .write = tmpfs_write,
    .read = tmpfs_read,
    .open = tmpfs_open,
    .close = tmpfs_close,
    .lseek64 = tmpfs_lseek64,
    .ioctl = tmpfs_ioctl
};

struct filesystem* tmpfs_init(struct vnode **root_node) {

    struct vnode *node;
    struct tmpfs_dir *dir;
    struct tmpfs_internal *internal;

    node = kmalloc(sizeof(struct vnode));
    dir  = kmalloc(sizeof(struct tmpfs_dir));
    internal = kmalloc(sizeof(struct tmpfs_internal));

    log_puts("[tmpfs_init] dir: 0x", TMPFS_DEBUG);
    log_puth(dir, TMPFS_DEBUG);
    log_puts("\n", TMPFS_DEBUG);

    dir->size = 0;

    internal->name[0] = '\0';
    internal->type = TMPFS_IS_DIR;
    internal->dir  = dir;
    internal->node = NULL;

    node->mount = NULL;
    node->v_ops = &tmpfs_v_ops;
    node->f_ops = &tmpfs_f_ops;
    node->internal = internal;

    *root_node = node;

    return &tmpfs;
}

