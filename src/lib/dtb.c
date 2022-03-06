#include "dtb.h"

void print_level (unsigned int level) {

    while (level) {
        uart_puts("  ");
        level--;
    }

    return;
}

void parse_dtb (struct fdt_header *fdt) {

    unsigned int *structure_ptr;
    unsigned int w;
    unsigned int state = 0;
    unsigned int bytes = 0;
    unsigned int level = 0;
    char *node_name;
    char *property;
    char *fdt_strings;
    struct fdt_prop prop;


    structure_ptr = (unsigned int *)((char *)fdt + SWAP_UINT32(fdt->off_dt_struct));
    fdt_strings  = ((char *)fdt) + SWAP_UINT32(fdt->off_dt_strings);
    uart_puts("( ~ )\n");

    while (1) {
        
        w = *structure_ptr;

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
                node_name = (char *)structure_ptr;
                bytes = strlen(node_name);
                
                if (bytes != 0) 
                {
                    bytes += 1;
                    print_level(level-1);
                    uart_puts("( ");
                    uart_puts(node_name);
                    uart_puts(" )\n");
                    structure_ptr = structure_ptr + ((bytes + 3) >> 2) - 1;
                }

            }
            else if (state == FDT_PROP)
            {
                prop.len     = SWAP_UINT32(*structure_ptr);
                structure_ptr++;
                prop.nameoff = SWAP_UINT32(*structure_ptr);
                structure_ptr++;

                property = (char *)structure_ptr;
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
                    structure_ptr = structure_ptr + ((bytes + 3) >> 2) - 1;
                }
            }
        }

        structure_ptr++;
    }

    return;
}
