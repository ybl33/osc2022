#include "uart.h"

int main () {

    // Backup DTB base address
    register unsigned long x0 asm ("x0");

    unsigned long kernel_base = 0x80000;
    unsigned int kernel_size = 0;
    unsigned long dtb_base = x0;
    char* kernel_ptr = (char *)kernel_base;

    uart_init();
    uart_putc(0x0C);
    uart_puts("[Uartboot] Waiting for kernel image.\n");
    
    for (int i = 0; i < 4; i++) {
        kernel_size = kernel_size << 8;
        kernel_size = kernel_size | uart_get();
    }

    uart_puts("Receiving Kernel image... (file size: 0x");
    uart_puth(kernel_size);
    uart_puts(" bytes)\n");

    for (int i = 0; i < kernel_size; i++) {
        kernel_ptr[i] = uart_get();
    }

    asm volatile ("mov x0, %0\n\t" : "=r" (dtb_base));
    asm volatile ("br %0\n\t" : "=r" (kernel_base));

    return 0;
}