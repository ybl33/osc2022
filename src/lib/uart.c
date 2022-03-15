#include "uart.h"

void uart_init () {
    unsigned int d;

    /* Get previous status of GPFSEL1 */
    d = mmio_get(GPFSEL1); 
    /* Clear GPIO14, GPIO15 setting */
    d = d & ~( (7 << GPIO14_GPFSEL_OFFSET) | (7 << GPIO15_GPFSEL_OFFSET) ); 
    /* Set GPIO14, GPIO15 to ALT5 */
    d = d | (2 << GPIO14_GPFSEL_OFFSET) | (2 << GPIO15_GPFSEL_OFFSET);
    mmio_put(GPFSEL1, d);
    /* Disable GPIO PULL UP/DOWN */
    mmio_put(GPPUD, 0);
    /* Wait 150 cycles (required set-up time) */
    for (int i = 0; i < 150; i++) asm volatile ("nop");
    /* Enable clock for setting GPIO14, GPIO15*/
    d = ( (1 << GPIO14_GPPUDCLK_OFFSET) | (1 << GPIO15_GPPUDCLK_OFFSET) );
    mmio_put(GPPUDCLK0, d);
    /* Wait 150 cycles (required set-up time) */
    for (int i = 0; i < 150; i++) asm volatile ("nop");
    /* Remove clock */
    mmio_put(GPPUDCLK0, 0);

    /* Set mini uart enable */
    d = mmio_get(AUX_ENABLES);
    d = d | 1;
    mmio_put(AUX_ENABLES, d);
    /* Disable transmitter and receiver during configuration */
    mmio_put(AUX_MU_CNTL_REG, 0);
    /* Disable interrupt */
    mmio_put(AUX_MU_IER_REG, 0);
    /* Set the data size to 8 bit */
    mmio_put(AUX_MU_LCR_REG, 3);
    /* Disable auto flow control */
    mmio_put(AUX_MU_MCR_REG, 0);
    /* Set baud rate to 115200 */
    mmio_put(AUX_MU_BAUD_REG, 270);
    /* Clear RX/TX FIFO */
    mmio_put(AUX_MU_IIR_REG, 0xC6);
    /* Enable transmitter and receiver */
    mmio_put(AUX_MU_CNTL_REG, 3);
    
    /* Clear rx data */
    uart_flush();

    return;
}

void uart_flush () {
    while (mmio_get(AUX_MU_LSR_REG) & AUX_MU_LSR_DATA_READY) mmio_get(AUX_MU_IO_REG);
}

char uart_get () {
    char c;
    /* Wait for data ready */
    while ((mmio_get(AUX_MU_LSR_REG) & AUX_MU_LSR_DATA_READY) == 0) asm volatile ("nop");
    c = mmio_get(AUX_MU_IO_REG);

    return c;
}

char uart_getc () {
    char c;
    
    c = uart_get();
    if (c == '\r') c = '\n';

    return c;
}

void uart_putc (char c) {
    /* Wait for transmitter ready to receive data */
    while ((mmio_get(AUX_MU_LSR_REG) & AUX_MU_LSR_TRANS_EMPTY) == 0) asm volatile ("nop");
    /* Put data */
    mmio_put(AUX_MU_IO_REG, c);

    return;
}

void uart_puth (unsigned int d) {
    unsigned int c;

    for (int i = 28; i >= 0; i -= 4) {
        /* Highest 4 bits */
        c = (d >> i) & 0xF;
        /* Translate to hex */
        c = (c > 9) ? (0x37 + c) : (0x30 + c) ;
        uart_putc(c);
    }

    return;
}

void uart_puts (char *s) {

    while (*s != '\0') {

        if (*s == '\n') 
            uart_putc('\r');
            
        uart_putc(*s++);

    }
    
    return;
}

