#ifndef __BUDDY__H__
#define __BUDDY__H__
#include "stddef.h"
#include "list.h"
#include "heap.h"
#include "log.h"

// From 0x10000000 to 0x20000000
#define BUDDY_BASE_ADDR       (0x00000000)
#define BUDDY_SIZE            (0x3C000000)

// Block size from 4KB to 128 MB
#define BUDDY_ORDER_NUM       (16)
#define BUDDY_PAGE_SIZE       (4096)
#define BUDDY_PAGE_NUM        (BUDDY_SIZE / BUDDY_PAGE_SIZE)

#define BUDDY_FREE            (0x0)
#define BUDDY_USED            (0x1)

#define BUDDY_LOG_ON          (1)

typedef struct page_status_t {
    unsigned char flag;
    unsigned char order;
} page_status_t;


void memory_reserve(unsigned long start, unsigned long end);
void *alloc_pages(unsigned int page_num);
void free_page(void *addr);
void dump_buddy_free_lists();

#endif