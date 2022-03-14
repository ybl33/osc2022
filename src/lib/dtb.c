#include "dtb.h"

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

void parse_dtb (struct fdt_header *fdt) {

    unsigned int w;
    unsigned int state = 0;
    unsigned int bytes = 0;
    unsigned int level = 0;
    char *fdt_structure;
    char *node_name;
    char *property;
    char *fdt_strings;
    struct fdt_prop prop;

    if (fdt->magic != FDT_HEADER_MAGIC)
    {
        uart_puts("Wrong fdt header magic,\n");
        return;
    }

    fdt_structure = (char *)fdt + SWAP_UINT32(fdt->off_dt_struct);
    fdt_strings   = (char *)fdt + SWAP_UINT32(fdt->off_dt_strings);
    uart_puts("( ~ )\n");

    while (1) {
        
        w = *(unsigned int *)fdt_structure;

        if (w == FDT_BEGIN_NODE)
        {
            level++;
            state = FDT_BEGIN_NODE;
            fdt_structure += 4;
        }
        else if (w == FDT_END_NODE)
        {
            state = FDT_END_NODE;
            fdt_structure += 4;
            level--;
        }
        else if (w == FDT_PROP)
        {
            state = FDT_PROP;
            fdt_structure += 4;
        }
        else if (w == FDT_NOP)
        {
            state = FDT_NOP;
            fdt_structure += 4;
        }
        else if (w == FDT_END) 
        {
            break;
        }


        if (state == FDT_BEGIN_NODE)
        {
            node_name = fdt_structure;
            bytes = strlen(node_name);
            
            if (bytes != 0)
            {
                print_level(level-1);
                uart_puts("( ");
                uart_puts(node_name);
                uart_puts(" )\n");
            }
            else
            {
                fdt_structure += 4;
            }

            fdt_structure = dtb_align(fdt_structure + bytes);
        } 
        else if (state == FDT_PROP)
        {
            prop.len     = SWAP_UINT32(*(unsigned int *)fdt_structure);
            fdt_structure += 4;
            prop.nameoff = SWAP_UINT32(*(unsigned int *)fdt_structure);
            fdt_structure += 4;

            property = fdt_structure;
            bytes    = prop.len;

            print_level(level);
            uart_puts("+ ");
            uart_puts(&fdt_strings[prop.nameoff]);
            uart_puts(": ");

            for (int i = 0; i < bytes; i++)
            {
                uart_putc(property[i]);
            }

            uart_puts("\n");                
            fdt_structure = dtb_align(fdt_structure + bytes);
        }
    
    }

    return;
}
