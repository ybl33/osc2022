#include "dtb.h"
static unsigned int level;
static void *fdt_structure;
static void *fdt_strings;
extern unsigned long CPIO_BASE;

char* dtb_align (char* v) {
    unsigned long lower_bits = (unsigned long)v & 0x3;

    v = v - lower_bits;

    if (lower_bits > 0)
    {
        v = v + 4;
    }

    return (char *)v;
}

void print_level (unsigned int level) {

    while (level) {
        uart_puts("  ");
        level--;
    }

    return;
}

void fdt_traverse (struct fdt_header *fdt, void (*callback) ()) {

    unsigned int w;
    unsigned int bytes = 0;
    char *fdt_ptr;

    if (fdt->magic != FDT_HEADER_MAGIC)
    {
        uart_puts("Wrong fdt header magic.\n");
        return;
    }

    fdt_structure = (char *)fdt + SWAP_UINT32(fdt->off_dt_struct);
    fdt_strings   = (char *)fdt + SWAP_UINT32(fdt->off_dt_strings);

    level   = 0;
    fdt_ptr = fdt_structure;

    while (1) {
        
        w = *(unsigned int *)fdt_ptr;

        if (w == FDT_BEGIN_NODE)
        {
            level++;
            callback(fdt_ptr);
            fdt_ptr += 4;
            bytes    = strlen(fdt_ptr);
            fdt_ptr  = dtb_align(fdt_ptr + bytes + 1);
        }
        else if (w == FDT_END_NODE)
        {
            level--;
            fdt_ptr += 4;
        }
        else if (w == FDT_PROP)
        {
            callback(fdt_ptr);
            fdt_ptr += 4;
            bytes   = SWAP_UINT32(*(unsigned int *)fdt_ptr);
            fdt_ptr = fdt_ptr + bytes + 8;
            fdt_ptr = dtb_align(fdt_ptr);
        }
        else if (w == FDT_NOP)
        {
            fdt_ptr += 4;
        }
        else if (w == FDT_END) 
        {
            break;
        }
    
    }

    return;
}

void lsdev_callback (void *fdt) {
    char* struct_prt;
    char* string_ptr;
    unsigned int state;
    unsigned int bytes;
    char *node_name;
    char *property;
    struct fdt_prop prop;

    struct_prt = (char *)fdt;
    string_ptr = (char *)fdt_strings;
    state      = *(unsigned int *)struct_prt;
    bytes      = 0;

    struct_prt += 4;

    if (state == FDT_BEGIN_NODE)
    {
        node_name = struct_prt;
        bytes = strlen(node_name);
        
        if (bytes != 0)
        {
            print_level(level-1);
            uart_puts("( ");
            uart_puts(node_name);
            uart_puts(" )\n");
        }

    } 
    else if (state == FDT_PROP)
    {
        prop.len     = SWAP_UINT32(*(unsigned int *)struct_prt);
        struct_prt += 4;
        prop.nameoff = SWAP_UINT32(*(unsigned int *)struct_prt);
        struct_prt += 4;

        property = struct_prt;
        bytes    = prop.len;

        print_level(level);
        uart_puts("+ ");
        uart_puts(&string_ptr[prop.nameoff]);
        uart_puts(": ");

        for (int i = 0; i < bytes; i++)
        {
            if (property[i] >= 32 && property[i] <= 126)
                uart_put(property[i]);
            else
                uart_put('_');
        }

        uart_puts("\n");                
    }

    return;
}

void initramfs_callback (void *fdt) {
    char* struct_prt;
    char* string_ptr;
    unsigned int state;
    char *property;
    struct fdt_prop prop;

    struct_prt = (char *)fdt;
    string_ptr = (char *)fdt_strings;
    state      = *(unsigned int *)struct_prt;

    struct_prt += 4;

    if (state == FDT_PROP)
    {

        prop.len     = SWAP_UINT32(*(unsigned int *)struct_prt);
        struct_prt += 4;
        prop.nameoff = SWAP_UINT32(*(unsigned int *)struct_prt);
        struct_prt += 4;

        property = struct_prt;

        if ( strcmp(&string_ptr[prop.nameoff], "linux,initrd-start") == 0 )
        {
            CPIO_BASE = SWAP_UINT32(*(unsigned int *)property);
        }

    }

    return;
}