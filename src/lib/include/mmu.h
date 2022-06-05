#ifndef __MMU__H__
#define __MMU__H__

#include "mmu_def.h"
#include "buddy.h"
#include "log.h"

#define PAGE_TABLE_SIZE            (4096)
#define PAGE_TABLE_ENTRY_NUM       (PAGE_TABLE_SIZE / 8)

#define MMU_DEBUG                  (0)

unsigned long va_to_pa (unsigned long va) ;
unsigned long pa_to_va (unsigned long pa) ;
unsigned long user_va_to_pa (unsigned long user_va) ;
unsigned long* create_page_table () ;
void page_table_init (unsigned long *page_table) ;
void set_page_table (unsigned long *page_table, unsigned long offset, unsigned long next_level, unsigned long attribute) ;
unsigned long* alloc_page_table (unsigned long *pgd, unsigned long va, unsigned long pa, unsigned attribute) ;
#endif