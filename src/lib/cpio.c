#include "cpio.h"

extern unsigned long DTB_BASE;
unsigned long CPIO_BASE;

unsigned long cpio_align (unsigned long v) {
    unsigned long lower_bits = v & 0x3;

    v = v - lower_bits;

    if (lower_bits > 0)
    {
        v = v + 4;
    }

    return v;
}

int cpio_header_parser (cpio_header_t *header, char** file_name, unsigned long* file_size, char** data, cpio_header_t **next_header) {
    
    unsigned long file_name_size;
    unsigned long tmp;
    
    /* Check header magic word */
    if (strncmp(header->c_magic, CPIO_HEADER_MAGIC, sizeof(header->c_magic)) != 0)
        return -1;
    
    /* Parse file_size and file_name */
    *file_size = htoin(header->c_filesize, sizeof(header->c_filesize));
    *file_name = ((char *)header) + CPIO_HEADER_SIZE;

    /* Cheak end of cpio */
    if (strncmp(*file_name, CPIO_FOOTER_MAGIC, sizeof(CPIO_FOOTER_MAGIC)) == 0)
        return 1;
    
    file_name_size = htoin(header->c_namesize, sizeof(header->c_namesize));

    tmp = (unsigned long)header + CPIO_HEADER_SIZE + file_name_size;
    tmp = cpio_align(tmp);
    *data = (char *)tmp;

    tmp = (unsigned long)*data + *file_size;
    tmp = cpio_align(tmp);
    *next_header = (cpio_header_t *)tmp;

    return 0;
}

int cpio_ls (cpio_header_t *header) {

    char* file_name;
    char* data;
    unsigned long file_size;
    unsigned int  end_of_cpio = 0;
    cpio_header_t *current_header;
    cpio_header_t *next_header;

    current_header = header;

    while (1) 
    {
        end_of_cpio = cpio_header_parser(current_header, &file_name, &file_size, &data, &next_header);
        if (end_of_cpio) break;

        current_header = next_header;
        uart_puts(file_name);
        uart_puts("\n");
    }

    return 0;
}

int cpio_cat (cpio_header_t *header, char* file_name) {
    
    char* header_file_name;
    char* data;
    unsigned long file_size;
    unsigned int  end_of_cpio = 0;
    cpio_header_t *current_header;
    cpio_header_t *next_header;

    current_header = header;

    while (1) 
    {
        end_of_cpio = cpio_header_parser(current_header, &header_file_name, &file_size, &data, &next_header);
        if (end_of_cpio) break;

        current_header = next_header;
        if (strcmp(file_name, header_file_name) == 0) 
        {
            for (int i = 0; i < file_size; i++) 
            {
                uart_put(data[i]);
                
                if (data[i] == '\n')
                {
                    uart_put('\r');
                }
            }
            return 0;
        }   
    }

    return -1;
}

void *cpio_load (cpio_header_t *header, char *file_name) {
    
    char *prog_base;
    char *header_file_name;
    char *data;
    unsigned long file_size;
    unsigned int  end_of_cpio = 0;
    cpio_header_t *current_header;
    cpio_header_t *next_header;

    current_header = header;

    while (1) 
    {
        end_of_cpio = cpio_header_parser(current_header, &header_file_name, &file_size, &data, &next_header);
        if (end_of_cpio) break;

        current_header = next_header;
        if (strcmp(file_name, header_file_name) == 0) 
        {
            prog_base = malloc(file_size);

            for (int i = 0; i < file_size; i++) 
            {
                prog_base[i] = data[i];
            }

            return (void *) prog_base;
        }   
    }

    return 0;
}

void cpio_init () {

    fdt_traverse((struct fdt_header *)DTB_BASE, initramfs_callback);

    return;
}