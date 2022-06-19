#include "cpiofs.h"

extern unsigned long CPIO_BASE;
bool cpiofs_mounted = false;
struct vnode cpiofs_root;

int cpiofs_mount(struct filesystem *fs, struct mount *mount) {

    struct vnode *node, *mount_node;
    struct cpiofs_internal *internal;
    char *name;

    if (cpiofs_mounted)
    {
        return -1;
    }

    mount_node = mount->root;
    mount_node->v_ops->getname(mount_node, &name);

    internal = cpiofs_root.internal;
    internal->name = name;

    mount_node->mount = mount;
    mount_node->v_ops = cpiofs_root.v_ops;
    mount_node->f_ops = cpiofs_root.f_ops;
    mount_node->internal = internal;

    cpiofs_mounted = true;

    return 0;
}

int cpiofs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name) {

    struct vnode *entry;
    struct cpiofs_internal *internal;
    struct cpiofs_dir *dir;
    char *entry_name;

    internal = dir_node->internal;

    if (internal->type == CPIOFS_IS_DIR) 
    {
        dir = &internal->dir;

        for (int i = 0; i < dir->size; i++)
        {
            entry = dir->nodes[i];
            entry->v_ops->getname(entry, &entry_name);

            if (!strcmp(entry_name, component_name))
            {

                log_puts("[cpiofs_lookup] entry_name: ", CPIOFS_DEBUG);
                log_puts(entry_name, CPIOFS_DEBUG);
                log_puts(", component_name: ", CPIOFS_DEBUG);
                log_puts(component_name, CPIOFS_DEBUG);
                log_puts("\n", CPIOFS_DEBUG);

                *target = entry;
                
                return 0;
            }

        }
    }

    return -1;
}

int cpiofs_create(struct vnode *dir_node, struct vnode **target, const char *component_name) {

    return -1;
}

int cpiofs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name) {

    return -1;
}

int cpiofs_getname(struct vnode *dir_node, const char **name) {

    struct cpiofs_internal *internal;

    internal = dir_node->internal;

    *name = internal->name;

    return 0;
}

int cpiofs_getsize(struct vnode *dir_node) {

    struct cpiofs_internal *internal;

    internal = dir_node->internal;

    return internal->file.size;
}

int cpiofs_write(struct file *file, const void *buf, size_t len) {

    return -1;
}

int cpiofs_read(struct file *file, void *buf, size_t len) {

    struct cpiofs_internal *internal;

    internal = file->vnode->internal;

    if (internal->type != CPIOFS_IS_FILE) 
    {
        return -1;
    }

    if (len > (internal->file.size - file->f_pos)) 
    {
        len = (internal->file.size - file->f_pos);
    }

    if (len == 0) 
    {
        return 0;
    }

    memcpy(buf, &internal->file.data[file->f_pos], len);

    file->f_pos += len;

    return len;
}

int cpiofs_open(struct vnode *file_node, struct file *target) {

    target->vnode = file_node;
    target->f_pos = 0;
    target->f_ops = file_node->f_ops;

    return 0;
}

int cpiofs_close(struct file *file) {

    file->vnode = NULL;
    file->f_pos = 0;
    file->f_ops = NULL;

    return 0;
}

long cpiofs_lseek64(struct file *file, long offset, int whence) {

    return -1;
}

int cpiofs_ioctl(struct file *file, unsigned long request, void *arg) {

    return -1;
}


struct filesystem cpiofs = {
    .name = "cpiofs",
    .mount = cpiofs_mount
};

struct vnode_operations cpiofs_v_ops = {
    .lookup = cpiofs_lookup,
    .create = cpiofs_create,
    .mkdir = cpiofs_mkdir,
    .getname = cpiofs_getname,
    .getsize = cpiofs_getsize
};

struct file_operations cpiofs_f_ops = {
    .write = cpiofs_write,
    .read = cpiofs_read,
    .open = cpiofs_open,
    .close = cpiofs_close,
    .lseek64 = cpiofs_lseek64,
    .ioctl = cpiofs_ioctl
};

void cpio_init_file (char *pathname, char *data, unsigned long size) {

    log_puts("Is a file\n", CPIOFS_DEBUG);

    struct vnode *dir_node, *new_node;
    struct cpiofs_internal *new_internal, *dir_internal;

    dir_node = get_dir_vnode(&cpiofs_root, &pathname);

    if (dir_node == NULL || pathname == NULL) 
    {
        return;
    }

    log_puts("dir_node: 0x", CPIOFS_DEBUG);
    log_puth(dir_node, CPIOFS_DEBUG);
    log_puts(", pathname: ", CPIOFS_DEBUG);
    log_puts(pathname, CPIOFS_DEBUG);

    dir_internal = dir_node->internal;

    if (dir_internal->type != CPIOFS_IS_DIR) 
    {
        return;
    }

    new_internal = kmalloc(sizeof(struct cpiofs_internal));
    new_node     = kmalloc(sizeof(struct vnode));

    new_internal->name      = pathname;
    new_internal->type      = CPIOFS_IS_FILE;
    new_internal->file.data = data;
    new_internal->file.size = size;
    new_internal->node      = new_node;

    new_node->mount    = NULL;
    new_node->v_ops    = &cpiofs_v_ops;
    new_node->f_ops    = &cpiofs_f_ops;
    new_node->parent   = dir_node;
    new_node->internal = new_internal;

    dir_internal->dir.nodes[dir_internal->dir.size] = new_node;
    dir_internal->dir.size++;

    log_puts(", dir.size: ", CPIOFS_DEBUG);
    log_putu(dir_internal->dir.size, CPIOFS_DEBUG);
    log_puts("\n", CPIOFS_DEBUG);

}

void cpio_init_dir (char *pathname) {
    
    log_puts("Is a dir\n", CPIOFS_DEBUG);

    struct vnode *dir_node, *new_node;
    struct cpiofs_internal *new_internal, *dir_internal;

    dir_node = get_dir_vnode(&cpiofs_root, &pathname);

    if (dir_node == NULL || pathname == NULL) 
    {
        return;
    }

    log_puts("dir_node: 0x", CPIOFS_DEBUG);
    log_puth(dir_node, CPIOFS_DEBUG);
    log_puts(", pathname: ", CPIOFS_DEBUG);
    log_puts(pathname, CPIOFS_DEBUG);

    dir_internal = dir_node->internal;

    if (dir_internal->type != CPIOFS_IS_DIR) 
    {
        return;
    }

    new_internal = kmalloc(sizeof(struct cpiofs_internal));
    new_node     = kmalloc(sizeof(struct vnode));

    new_internal->name      = pathname;
    new_internal->type      = CPIOFS_IS_DIR;
    new_internal->dir.size  = 0;
    new_internal->node      = new_node;

    new_node->mount    = NULL;
    new_node->v_ops    = &cpiofs_v_ops;
    new_node->f_ops    = &cpiofs_f_ops;
    new_node->parent   = dir_node;
    new_node->internal = new_internal;

    dir_internal->dir.nodes[dir_internal->dir.size] = new_node;
    dir_internal->dir.size++;

    log_puts(", dir.size: ", CPIOFS_DEBUG);
    log_putu(dir_internal->dir.size, CPIOFS_DEBUG);
    log_puts("\n", CPIOFS_DEBUG);

}

struct filesystem* cpiofs_init () {

    struct cpiofs_internal *internal;

    char* file_name;
    char* data;
    unsigned long file_size;
    unsigned int file_type;
    unsigned int end_of_cpio = 0;
    cpio_header_t *current_header;
    cpio_header_t *next_header;
    
    internal = kmalloc(sizeof(struct cpiofs_internal));

    internal->name = "\0";
    internal->type = CPIOFS_IS_DIR;
    internal->node = &cpiofs_root;
    internal->dir.size = 0;

    cpiofs_root.mount = NULL;
    cpiofs_root.v_ops = &cpiofs_v_ops;
    cpiofs_root.f_ops = &cpiofs_f_ops;
    cpiofs_root.parent = NULL;
    cpiofs_root.internal = internal;

    current_header = CPIO_BASE;

    // Skip .
    cpio_header_parser(current_header, &file_name, &file_size, &data, &next_header);
    current_header = next_header;

    while (1) 
    {
        end_of_cpio = cpio_header_parser(current_header, &file_name, &file_size, &data, &next_header);
        
        if (end_of_cpio)
        {
            break;
        }

        file_type = htoin(current_header->c_mode, 8) & CPIO_MODE_MASK;

        switch (file_type)
        {
            case CPIO_TYPE_DIR:
                cpio_init_dir(file_name);
                break;
            case CPIO_TYPE_FILE:
                cpio_init_file(file_name, data, file_size);
                break;
            default:
                break;
        }

        current_header = next_header;
    }

    return &cpiofs;
}