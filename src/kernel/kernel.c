#include "shell.h"
#include "uart.h"
#include "buddy.h"
#include "thread.h"

unsigned long DTB_BASE;
extern unsigned long __start;
extern unsigned long __end;
extern unsigned long CPIO_BASE;
static unsigned long kernel_start = (unsigned long) &__start;
static unsigned long kernel_end   = (unsigned long) &__end;

void kernel_init () {

    // System init
    uart_init();
    asyn_uart_init();
    cpio_init();

    // Reserve memory
    unsigned int dtb_size = SWAP_UINT32(((struct fdt_header *)DTB_BASE)->totalsize);
    memory_reserve(0xFFFF000000000000, 0xFFFF000000001000);                // Spin tables for multicore boot
    memory_reserve(kernel_start, kernel_end);          // Kernel image
    memory_reserve(CPIO_BASE, CPIO_BASE + 0x100000);   // initramfs
    memory_reserve(DTB_BASE, DTB_BASE + dtb_size);     // dtb 8KB

}


int main (unsigned long x0) {

    DTB_BASE = x0;

    kernel_init();
    shell_start();

    return 0;
}