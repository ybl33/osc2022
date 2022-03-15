#ifndef __DTB__H__
#define __DTB__H__
#include "cpio.h"
#include "stddef.h"
#include "string.h"
#include "uart.h"
#define FDT_HEADER_MAGIC (0xEDFE0DD0)
#define FDT_BEGIN_NODE   (0x01000000)
#define FDT_END_NODE     (0x02000000)
#define FDT_PROP         (0x03000000)
#define FDT_NOP          (0x04000000)
#define FDT_END          (0x09000000)

#define SWAP_UINT32(x)   (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))

struct fdt_header {
    unsigned int magic;
    unsigned int totalsize;
    unsigned int off_dt_struct;
    unsigned int off_dt_strings;
    unsigned int off_mem_rsvmap;
    unsigned int version;
    unsigned int last_comp_version;
    unsigned int boot_cpuid_phys;
    unsigned int size_dt_strings;
    unsigned int size_dt_struct;
};

struct fdt_prop {
    unsigned int len;
    unsigned int nameoff;
};

void print_level(unsigned int level);
void fdt_traverse(struct fdt_header *fdt, void (*callback) ());
void lsdev_callback(void *fdt);
void initramfs_callback(void *fdt);
#endif