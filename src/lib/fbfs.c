#include "fbfs.h"

bool fb_is_opened = false;
bool fb_is_inited = false;
static unsigned int __attribute__((aligned(0x10))) mbox[36];

int fbfs_mount(struct filesystem *fs, struct mount *mount) {

    struct vnode *mount_node;
    struct fbfs_internal *internal;
    char *name;

    internal       = kmalloc(sizeof(struct fbfs_internal));
    internal->node = kmalloc(sizeof(struct vnode));

    mount_node = mount->root;

    mount_node->v_ops->getname(mount_node, &name);

    internal->name           = name;
    internal->node->mount    = mount_node->mount;
    internal->node->v_ops    = mount_node->v_ops;
    internal->node->f_ops    = mount_node->f_ops;
    internal->node->parent   = mount_node->parent;
    internal->node->internal = mount_node->internal;
    internal->lfb            = NULL;
    internal->lfbsize        = 0;

    fb_is_opened = false;
    fb_is_inited = false;

    mount_node->mount    = mount;
    mount_node->v_ops    = &fbfs_v_ops;
    mount_node->f_ops    = &fbfs_f_ops;
    mount_node->internal = internal;

    return 0;
}

int fbfs_lookup (struct vnode *dir_node, struct vnode **target, const char *component_name) {

    return -1;
}

int fbfs_create (struct vnode *dir_node, struct vnode **target, const char *component_name) {

    return -1;
}

int fbfs_mkdir (struct vnode *dir_node, struct vnode **target, const char *component_name) {

    return -1;
}

int fbfs_getname (struct vnode *dir_node, const char **name) {

    struct fbfs_internal *internal;

    internal = dir_node->internal;

    *name = internal->name;

    return 0;
}

int fbfs_write(struct file *file, const void *buf, size_t len) {

    struct fbfs_internal *internal;

    internal = file->vnode->internal;

    if (!fb_is_inited) 
    {
        return -1;
    }

    if ((file->f_pos + len) > internal->lfbsize) 
    {
        return -1;
    }

    memcpy((internal->lfb + file->f_pos), buf, len);

    file->f_pos += len;

    return len;
}

int fbfs_read(struct file *file, void *buf, size_t len) {

    return -1;
}

int fbfs_open(struct vnode *file_node, struct file *target) {

    struct fbfs_internal *internal;

    internal = file_node->internal;

    if (fb_is_opened) 
    {
        return -1;
    }

    fb_is_opened = true;

    target->vnode = file_node;
    target->f_pos = 0;
    target->f_ops = file_node->f_ops;

    return 0;
}

int fbfs_close(struct file *file) {

    struct fbfs_internal *internal;

    internal = file->vnode->internal;

    file->vnode = NULL;
    file->f_pos = 0;
    file->f_ops = NULL;

    fb_is_opened = false;

    return 0;
}

long fbfs_lseek64(struct file *file, long offset, int whence) {
    struct fbfs_internal *internal;
    int base;

    internal = file->vnode->internal;
    
    switch (whence) 
    {
        case SEEK_SET:
            base = 0;
            break;
        case SEEK_CUR:
            base = file->f_pos;
            break;
        case SEEK_END:
            base = internal->lfbsize;
        default:
            return -1;
    }
    
    if (base + offset > internal->lfbsize) {
        return -1;
    }

    file->f_pos = base + offset;

    return 0;
}

int fbfs_ioctl(struct file *file, unsigned long request, void *arg) {

    struct fb_info *user_fb_info;
    struct fbfs_internal *internal;
    unsigned int width, height, pitch, isrgb;
    unsigned int message;

    if (request != 0) 
    {
        return -1;
    }

    internal = file->vnode->internal;

    mbox[0] = 35 * 4;
    mbox[1] = REQUEST_CODE;

    mbox[2] = 0x48003;  // set phy wh
    mbox[3] = 8;
    mbox[4] = 8;
    mbox[5] = 1024;     // FrameBufferInfo.width
    mbox[6] = 768;      // FrameBufferInfo.height

    mbox[7] = 0x48004;  // set virt wh
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = 1024;    // FrameBufferInfo.virtual_width
    mbox[11] = 768;     // FrameBufferInfo.virtual_height

    mbox[12] = 0x48009; // set virt offset
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0;       // FrameBufferInfo.x_offset
    mbox[16] = 0;       // FrameBufferInfo.y.offset

    mbox[17] = 0x48005; // set depth
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32;      // FrameBufferInfo.depth

    mbox[21] = 0x48006; // set pixel order
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1;       // RGB, not BGR preferably

    mbox[25] = 0x40001; // get framebuffer, gets alignment on request
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096;    // FrameBufferInfo.pointer
    mbox[29] = 0;       // FrameBufferInfo.size

    mbox[30] = 0x40008; // get pitch
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0;       // FrameBufferInfo.pitch

    mbox[34] = END_TAG;

    // this might not return exactly what we asked for, could be
    // the closest supported resolution instead
    
    /* Wait for mbox not full */
    while (mmio_get(MBOX_STATUS) & MBOX_FULL)
    {
        asm volatile ("nop");
    }

    /* Write the address of message to the mbox with channel identifier */
    message = (((unsigned int)(unsigned long)va_to_pa(&mbox)) & ~0xF) | (MBOX_CH_PROP & 0xF);
    mmio_put(MBOX_WRITE, message);

    /* Wait for our mbox response */
    do {

        while (mmio_get(MBOX_STATUS) & MBOX_EMPTY) asm volatile("nop");

    } while (mmio_get(MBOX_READ) != message);


    if (mbox[20] == 32 && mbox[28] != 0) 
    {
        mbox[28] &= 0x3FFFFFFF; // convert GPU address to ARM address
        width  = mbox[5];       // get actual physical width
        height = mbox[6];       // get actual physical height
        pitch  = mbox[33];      // get number of bytes per line
        isrgb  = mbox[24];      // get the actual channel order
        internal->lfb = (void *)pa_to_va(mbox[28]);
        internal->lfbsize = mbox[29];
    } 
    else 
    {
        // Unable to set screen resolution to 1024x768x32
        return -1;
    }
    
    user_fb_info = arg;

    user_fb_info->width  = width;
    user_fb_info->height = height;
    user_fb_info->pitch  = pitch;
    user_fb_info->isrgb  = isrgb;

    fb_is_inited = true;

    return 0;
}


struct filesystem fbfs = {
    .name = "fbfs",
    .mount = fbfs_mount
};

struct vnode_operations fbfs_v_ops = {
    .lookup = fbfs_lookup,
    .create = fbfs_create,
    .mkdir = fbfs_mkdir,
    .getname = fbfs_getname
};

struct file_operations fbfs_f_ops = {
    .write   = fbfs_write,
    .read    = fbfs_read,
    .open    = fbfs_open,
    .close   = fbfs_close,
    .lseek64 = fbfs_lseek64,
    .ioctl   = fbfs_ioctl
};

struct filesystem *fbfs_init () {

    return &fbfs;
}