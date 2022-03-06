#include "dtb.h"

void print_level (unsigned int level) {

    while (level) {
        uart_puts("  ");
        level--;
    }

    return;
}

void parse_dtb (struct fdt_header *fdt) {

    unsigned int *fdt_structure;
    unsigned int w;
    unsigned int state = 0;
    unsigned int bytes = 0;
    unsigned int level = 0;
    char *node_name;
    char *property;
    char *fdt_strings;
    struct fdt_prop prop;

    if (fdt->magic != FDT_HEADER_MAGIC)
    {
        uart_puts("Wrong fdt header magic,\n");
        return;
    }

    fdt_structure = (unsigned int *)((char *)fdt + SWAP_UINT32(fdt->off_dt_struct));
    fdt_strings   = ((char *)fdt) + SWAP_UINT32(fdt->off_dt_strings);
    uart_puts("( ~ )\n");

    while (1) {
        
        w = *fdt_structure;

        if (w == FDT_BEGIN_NODE)
        {
            level++;
            state = FDT_BEGIN_NODE;
        }
        else if (w == FDT_END_NODE)
        {
            level--;
            state = FDT_END_NODE;
        }
        else if (w == FDT_PROP)
        {
            state = FDT_PROP;
        }
        else if (w == FDT_NOP)
        {
            state = FDT_NOP;
        }
        else if (w == FDT_END) 
        {
            break;
        }
        else 
        {
            if (state == FDT_BEGIN_NODE)
            {
                node_name = (char *)fdt_structure;
                bytes = strlen(node_name);
                
                if (bytes != 0) 
                {
                    bytes += 1;
                    print_level(level-1);
                    uart_puts("( ");
                    uart_puts(node_name);
                    uart_puts(" )\n");
                    fdt_structure = fdt_structure + ((bytes + 3) >> 2) - 1;
                }

            }
            else if (state == FDT_PROP)
            {
                prop.len     = SWAP_UINT32(*fdt_structure);
                fdt_structure++;
                prop.nameoff = SWAP_UINT32(*fdt_structure);
                fdt_structure++;

                property = (char *)fdt_structure;
                bytes    = prop.len;

                if (bytes != 0) 
                {
                    print_level(level);
                    uart_puts("+ ");
                    uart_puts(&fdt_strings[prop.nameoff]);
                    uart_puts(": ");

                    for (int i = 0; i < bytes - 1; i++)
                    {
                        uart_putc(property[i]);
                    }

                    uart_puts("\n");
                    fdt_structure = fdt_structure + ((bytes + 3) >> 2) - 1;
                }
            }
        }

        fdt_structure++;
    }

    return;
}
