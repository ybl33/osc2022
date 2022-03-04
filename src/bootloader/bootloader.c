#include "uart.h"

int main () {
    char *kernel_p = (char *)0x80000;
    unsigned int kernel_size = 0;
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
        kernel_p[i] = uart_get();
    }

    void (*kernel)(void) = (void *)kernel_p;

    kernel();
    
    return 0;
}