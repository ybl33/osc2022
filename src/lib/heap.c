#include "heap.h"

extern unsigned long __heap_start; /* declared in the linker script */
extern unsigned long __heap_size;  /* declared in the linker script */
static unsigned long heap_top  = (unsigned long) &__heap_start;
static unsigned long heap_size = (unsigned long) &__heap_size;
static unsigned long *curr_top = (unsigned long *) 0xFFFFFFF0, *heap_end = (unsigned long *) 0xFFFFFFF0;

void *malloc (size_t n) {

    void *return_ptr;

    if (curr_top == heap_end) // first time to call malloc()?
    {
        curr_top  = (unsigned long *) heap_top;
        heap_end  = (unsigned long *) ((heap_top + heap_size) & 0xFFFFFFF0);
        *curr_top = (unsigned long) heap_end;
    }

    return_ptr = curr_top;
    curr_top   = (unsigned long *)((char *)curr_top + n);

    return return_ptr;
}

void free (void *m) {

    return;
}

void *calloc (size_t n, size_t sz) {
    
    void *return_ptr;

    return_ptr = malloc(n * sz);
    memset(return_ptr, 0, n * sz);

    return return_ptr;
}