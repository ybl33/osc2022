#include "mmu.h"

unsigned long va_to_pa (unsigned long va) {

    unsigned long pa;

    pa = va & 0x0000ffffffffffff;

    return pa;
}

unsigned long pa_to_va (unsigned long pa) {

    unsigned long va;

    va = pa | 0xffff000000000000;

    return va;
}

unsigned long user_va_to_pa (unsigned long user_va) {

    unsigned long offset;
    unsigned long *curr_table;
    unsigned long pd_type;
    unsigned long pa = 0;

    asm volatile ("mrs %0, ttbr0_el1" : "=r" (curr_table));
    curr_table = pa_to_va(curr_table);

    log_puts("[va_user_to_kernel] curr_table: ", MMU_DEBUG);
    log_puth((unsigned long)curr_table >> 32, MMU_DEBUG);
    log_puth((unsigned long)curr_table, MMU_DEBUG);
    log_puts("\n", MMU_DEBUG);

    for (int level = 3; level >= 0; level--)
    {
        offset     = (user_va >> (12 + level * 9)) & 0x1FF;
        pd_type    = (curr_table[offset] & 3);

        if (pd_type == PD_TABLE)
        {
            curr_table = (unsigned long *)(pa_to_va(curr_table[offset] & ~0xfff));
        }
        else if (pd_type == PD_BLOCK)
        {
            curr_table = (unsigned long *)(pa_to_va(curr_table[offset] & ~0xfff));
            break;
        }
        else
        {
            log_puts("[va_user_to_kernel] Error, encounter an invalid entry.\n", MMU_DEBUG);
        }

    }

    pa = ((unsigned long)curr_table) | (user_va & 0xFFF);
    pa = va_to_pa(pa);

    log_puts("[va_user_to_kernel] user_va: ", MMU_DEBUG);
    log_puth(user_va >> 32, MMU_DEBUG);
    log_puth(user_va, MMU_DEBUG);
    log_puts(", pa: ", MMU_DEBUG);
    log_puth(pa >> 32, MMU_DEBUG);
    log_puth(pa, MMU_DEBUG);
    log_puts("\n", MMU_DEBUG);

    return pa;
}

unsigned long* create_page_table () {

    unsigned long *page_table = alloc_pages(1);

    page_table_init(page_table);

    return page_table;
}

void page_table_init (unsigned long *page_table) {

    for (int i = 0; i < PAGE_TABLE_ENTRY_NUM; i++)
    {
        page_table[i] = 0;
    }

    log_puts("[page_table_init] *page_table: 0x", MMU_DEBUG);
    log_puth((unsigned long)page_table >> 32, MMU_DEBUG);
    log_puth((unsigned long)page_table, MMU_DEBUG);
    log_puts("\n", MMU_DEBUG);

    return;
}

void set_page_table (unsigned long *page_table, unsigned long offset, unsigned long next_level, unsigned long attribute) {

    page_table[offset] = next_level | attribute;

}

unsigned long* alloc_page_table (unsigned long *pgd, unsigned long va, unsigned long pa, unsigned attribute) {

    unsigned long offset;
    unsigned long *curr_table = pgd;
    unsigned long *page_va;

    // Four level page table: 
    //     - level = 3 : PGD -> PUD
    //     - level = 2 : PUD -> PMD
    //     - level = 1 : PMD -> PTE
    for (int level = 3; level > 0; level--)
    {
        offset = (va >> (12 + level * 9)) & 0x1FF; // 9 bits

        if ((curr_table[offset] & 1) == 0) // entry not alloc yet
        {
            // alloc a table for this entry
            curr_table[offset] = va_to_pa((unsigned long)create_page_table());
            // set attribute
            curr_table[offset] = curr_table[offset] | PD_TABLE;
        }

        // move to next level
        curr_table = (unsigned long *)(pa_to_va(curr_table[offset] & ~0xfff));
    }

    // PTE -> physical page
    offset = (va >> 12) & 0x1FF;

    if (pa)
    {
        curr_table[offset] = pa | attribute | PD_PAGE;
        page_va = (unsigned long *)pa_to_va(pa);
    }
    else
    {
        // If no physical page provided, allocate a page
        page_va = alloc_pages(1);
        curr_table[offset] = va_to_pa((unsigned long)page_va) | attribute | PD_PAGE;
    }

    log_puts("[alloc_page_table] pgd: 0x", MMU_DEBUG);
    log_puth((unsigned long)pgd >> 32, MMU_DEBUG);
    log_puth((unsigned long)pgd, MMU_DEBUG);
    log_puts(", pa: 0x", MMU_DEBUG);
    log_puth((unsigned long)pa_to_va(page_va) >> 32, MMU_DEBUG);
    log_puth((unsigned long)pa_to_va(page_va), MMU_DEBUG);
    log_puts("\n", MMU_DEBUG);
    return page_va;
}