#include "uart.h"

int main () {

    unsigned int kernel_size = 0;
    unsigned long kernel_base = 0x80000;
    volatile unsigned long dtb_base;
    char* kernel_ptr = (char *)kernel_base;

    // Backup DTB base address
    asm volatile ("mov %0, x0" : "=r" (dtb_base));

    // Read kernel
    uart_init();
    uart_put(0x0C);
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

    asm volatile ("mov x1, %0" : : "r" (kernel_base));
    asm volatile ("mov x0, %0" : : "r" (dtb_base));
    asm volatile ("br x1");

    return 0;
}