#include "buddy.h"

page_status_t *buddy_status = NULL;
list_t *free_lists = NULL;

void buddy_status_init () {

    buddy_status = malloc(sizeof(page_status_t) * BUDDY_PAGE_NUM);
    
    for (int i = 0; i < BUDDY_PAGE_NUM; i++)
    {
        buddy_status[i].flag  = BUDDY_FREE;
        buddy_status[i].order = 0;
    }

    return;
}

unsigned int page_num_to_order (unsigned int page_num) {

    unsigned int order;

    // log2(page_num)
    order = (sizeof(int) * 8 - __builtin_clz(page_num) - 1);

    return order;
}

unsigned int addr_to_page_index (void* addr) {

    unsigned int index;

    index = ((unsigned long)addr - BUDDY_BASE_ADDR) / BUDDY_PAGE_SIZE;

    return index;
}

void* page_index_to_addr (unsigned int index) {

    void* addr;

    addr = (void *)((unsigned long)index * BUDDY_PAGE_SIZE + BUDDY_BASE_ADDR);

    return addr;
}

unsigned int get_buddy_index (unsigned int index, unsigned int order) {

    unsigned int buddy_index;

    buddy_index = index ^ (1 << order);

    return buddy_index;
}

void buddy_free_list_init () {

    unsigned int c_index, b_index;

    if (buddy_status == NULL)
    {
        buddy_status_init();
    }

    free_lists = malloc(sizeof(list_t) * BUDDY_ORDER_NUM);

    for (int order = 0; order < BUDDY_ORDER_NUM; order++) 
    {
        list_init(&free_lists[order]);
    }

    for (int order = 0; order < (BUDDY_ORDER_NUM - 1); order++) 
    {
        list_init(&free_lists[order]);

        c_index = 0;

        while (1)
        {
            b_index = get_buddy_index(c_index, order);

            if (c_index > BUDDY_PAGE_NUM || b_index > BUDDY_PAGE_NUM)
            {
                break;
            }

            if (buddy_status[c_index].flag  == BUDDY_FREE && 
                buddy_status[b_index].flag  == BUDDY_FREE &&
                buddy_status[c_index].order == order && 
                buddy_status[b_index].order == order)
            {
                buddy_status[c_index].order += 1;
            }

            c_index = c_index + (1 << (order + 1));
        }

    }

    for (int i = 0, order; i < BUDDY_PAGE_NUM; i += (1 << order)) 
    {
        order = buddy_status[i].order;
        if (buddy_status[i].flag == BUDDY_FREE)
        {
            log_puts("[buddy free list init] start_addr: 0x", BUDDY_LOG_ON);
            log_puth((unsigned long)page_index_to_addr(i), BUDDY_LOG_ON);
            log_puts(", end_addr: 0x", BUDDY_LOG_ON);
            log_puth((unsigned long)page_index_to_addr(i + (1 << order)) - 1, BUDDY_LOG_ON);
            log_puts(", order: ", BUDDY_LOG_ON);
            log_putu(order, BUDDY_LOG_ON);
            log_puts("\n", BUDDY_LOG_ON);

            // Add the memory into free list
            list_push(&free_lists[order], (list_t *)page_index_to_addr(i));

        }
    }

    return;
}


void memory_reserve(unsigned long start, unsigned long end) {

    if (buddy_status == NULL)
    {
        buddy_status_init();
    }

    unsigned long start_page_idx = start / BUDDY_PAGE_SIZE;
    unsigned long end_page_idx   = (end + BUDDY_PAGE_SIZE - 1) / BUDDY_PAGE_SIZE;

    for (int i = start_page_idx; i < end_page_idx; i++)
    {
        buddy_status[i].flag = BUDDY_USED;
    }

    log_puts("[mem reserve] start: 0x", BUDDY_LOG_ON);
    log_puth(start, BUDDY_LOG_ON);
    log_puts(", end: 0x", BUDDY_LOG_ON);
    log_puth(end, BUDDY_LOG_ON);
    log_puts("\n", BUDDY_LOG_ON);

    return;
}


void *alloc_pages (unsigned int page_num) {

    unsigned int order = page_num_to_order(page_num);
    unsigned int fb_order;
    unsigned int index, buddy_index;
    void *ptr = NULL;

    if (free_lists == NULL)
    {
        buddy_free_list_init();
    }

    if (order > (BUDDY_ORDER_NUM - 1))
    {
        return NULL;
    }

    for (fb_order = order; fb_order < BUDDY_ORDER_NUM; fb_order++)
    {
        ptr = list_pop(&free_lists[fb_order]);

        if (ptr != NULL)
        {
            // Found a free block
            log_puts("[buddy alloc] Found a free block, address: 0x", BUDDY_LOG_ON);
            log_puth((unsigned long)ptr, BUDDY_LOG_ON);
            log_puts(", order: ", BUDDY_LOG_ON);
            log_putu(fb_order, BUDDY_LOG_ON);
            log_puts(".\n", BUDDY_LOG_ON);

            break;
        }
    }

    if (ptr == NULL)
    {
        // No more memory
        return NULL;
    }

    index = addr_to_page_index(ptr);

    while (fb_order > order)
    {
        // Split the free block, put the right child back into free list
        fb_order = fb_order - 1;
        buddy_index = get_buddy_index(index, fb_order);

        buddy_status[buddy_index].flag  = BUDDY_FREE;
        buddy_status[buddy_index].order = fb_order;

        list_push(&free_lists[fb_order], (list_t *)page_index_to_addr(buddy_index));

        log_puts("[buddy split] Put the buddy into free list, address: 0x", BUDDY_LOG_ON);
        log_puth((unsigned long)page_index_to_addr(buddy_index), BUDDY_LOG_ON);
        log_puts(", order: ", BUDDY_LOG_ON);
        log_putu(fb_order, BUDDY_LOG_ON);
        log_puts(".\n", BUDDY_LOG_ON);
    }

    buddy_status[index].flag  = BUDDY_USED;
    buddy_status[index].order = order;

    dump_buddy_free_lists();

    return ptr;
}

void free_page (void *addr) {

    unsigned int b_index;
    unsigned int index;
    unsigned int order;
    
    if ((unsigned long)addr < BUDDY_BASE_ADDR || (unsigned long)addr > (BUDDY_BASE_ADDR + BUDDY_SIZE))
    {
        return;
    }

    index = addr_to_page_index(addr);
    order = buddy_status[index].order;

    while (order <= BUDDY_ORDER_NUM)
    {
        b_index = get_buddy_index(index, order);

        if (buddy_status[b_index].flag == BUDDY_FREE && buddy_status[b_index].order == order)
        {
            log_puts("[buddy merge] Buddy is free, do the merge, address: 0x", BUDDY_LOG_ON);
            log_puth((unsigned long)page_index_to_addr(b_index), BUDDY_LOG_ON);
            log_puts(", order: ", BUDDY_LOG_ON);
            log_putu(order, BUDDY_LOG_ON);
            log_puts(".\n", BUDDY_LOG_ON);

            list_remove(page_index_to_addr(b_index));
            order += 1;
        }
        else
        {
            break;
        }

    }

    buddy_status[index].flag  = BUDDY_FREE;
    buddy_status[index].order = order;

    list_push(&free_lists[order], (list_t *)page_index_to_addr(index));

    log_puts("[buddy free] Free the memory, address: 0x", BUDDY_LOG_ON);
    log_puth((unsigned long)page_index_to_addr(index), BUDDY_LOG_ON);
    log_puts(", order: ", BUDDY_LOG_ON);
    log_putu(order, BUDDY_LOG_ON);
    log_puts(".\n", BUDDY_LOG_ON);
    dump_buddy_free_lists();

    return;
}


void dump_buddy_free_lists () {

    list_t *head, *ptr;

    log_puts("+-----------------------------\n", BUDDY_LOG_ON);
    log_puts("| Buddy system free lists\n", BUDDY_LOG_ON);

    if (free_lists != NULL)
    {
        for (int i = 0; i < BUDDY_ORDER_NUM; i++)
        {
            head = ptr = &free_lists[i];

            if (ptr != ptr->prev)
            {
                log_puts("|    - free blocks of order ", BUDDY_LOG_ON);
                log_putu(i, BUDDY_LOG_ON);
                log_puts(" : ", BUDDY_LOG_ON);

                ptr = ptr->next;

                while (ptr != head)
                {
                    log_puts("0x", BUDDY_LOG_ON);
                    log_puth((unsigned long)ptr, BUDDY_LOG_ON);
                    log_puts(" -> ", BUDDY_LOG_ON);
                    ptr = ptr->next;
                }

                log_puts("\b\b\b   \n", BUDDY_LOG_ON);
            }
        }
    }

    log_puts("+-----------------------------\n", BUDDY_LOG_ON);

    return;
}