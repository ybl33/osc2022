#ifndef __CPIO__H__
#define __CPIO__H__
#include "dtb.h"
#include "uart.h"
#include "string.h"

/* CPIO new ASCII header */
typedef struct cpio_header_t {
    char	   c_magic[6];
    char	   c_ino[8];
    char	   c_mode[8];
    char	   c_uid[8];
    char	   c_gid[8];
    char	   c_nlink[8];
    char	   c_mtime[8];
    char	   c_filesize[8];
    char	   c_devmajor[8];
    char	   c_devminor[8];
    char	   c_rdevmajor[8];
    char	   c_rdevminor[8];
    char	   c_namesize[8];
    char	   c_check[8];
} cpio_header_t;

#define CPIO_HEADER_MAGIC "070701"
#define CPIO_FOOTER_MAGIC "TRAILER!!!"
#define CPIO_HEADER_SIZE  (sizeof(cpio_header_t))

unsigned long cpio_align(unsigned long v);
unsigned long htoin(char *s, unsigned int n);
int cpio_header_parser(cpio_header_t *header, char** file_name, unsigned long* file_size, char** data, cpio_header_t **next_header);
int cpio_ls(cpio_header_t *header);
int cpio_cat(cpio_header_t *header, char* file_name);
void cpio_init();

#endif