#include "uart.h"

/* Asynchronous Read and Write */
char read_buffer[READ_BUF_SIZE];
char write_buffer[WRITE_BUF_SIZE];

unsigned int read_head;
unsigned int write_head;
unsigned int read_tail;
unsigned int write_tail;

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
    mmio_put(AUX_MU_IIR_REG, 0x6);
    /* Enable transmitter and receiver */
    mmio_put(AUX_MU_CNTL_REG, 3);
    
    /* Clear rx data */
    uart_flush();

    return;
}


bool uart_rx_valid () {
    return (mmio_get(AUX_MU_LSR_REG) & AUX_MU_LSR_DATA_READY);
}

bool uart_tx_ready () {
    return (mmio_get(AUX_MU_LSR_REG) & AUX_MU_LSR_TRANS_EMPTY);
}

void uart_flush () {
    while (uart_rx_valid()) mmio_get(AUX_MU_IO_REG);
}

char uart_get () {

    char c;
    
    /* Wait for data ready */
    while (!uart_rx_valid()) asm volatile ("nop");
    c = mmio_get(AUX_MU_IO_REG);

    return c;
}

char uart_getc () {
    char c;
    
    c = uart_get();
    if (c == '\r') c = '\n';

    return c;
}

void uart_put (char c) {

    /* Wait for transmitter ready to receive data */
    while (!uart_tx_ready()) asm volatile ("nop");
    
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
        uart_put(c);
    }

    return;
}

void uart_putu (unsigned int d) {

    char c;
    bool leading_zero = true;
    unsigned int div = 1000000000;
    unsigned int digit;

    if (d == 0)
    {
        uart_put('0');
        return;
    }

    while (div)
    {

        digit = d / div;

        if (digit) 
        {
            leading_zero = false;
            d = d - digit * div;
        }

        if (!leading_zero)
        {
            c = '0' + digit;
            uart_put(c);
        }

        div   = div / 10;

    }

    return;
}

void uart_puts (char *s) {

    while (*s != '\0') {

        if (*s == '\n') 
            uart_put('\r');
            
        uart_put(*s++);

    }
    
    return;
}

void asyn_uart_init () {

    /* Enable RX interrupt */
    set_aux_int(true);
    set_uart_rx_int(true);
    set_uart_tx_int(false);

    /* Enable AUX interrupt */

    read_head  = 0;
    write_head = 0;
    read_tail  = 0;
    write_tail = 0;

    return;
}

void set_uart_rx_int (bool enable) {

    unsigned int d;
    d = mmio_get(AUX_MU_IER_REG);

    if (enable)
    {
        d = d | 0x1;
    }
    else
    {
        d = d & ~(0x1);
    }

    mmio_put(AUX_MU_IER_REG, d);

    return;
}

void set_uart_tx_int (bool enable) {

    unsigned int d;
    d = mmio_get(AUX_MU_IER_REG);

    if (enable)
    {
        d = d | 0x2;
    }
    else
    {
        d = d & ~(0x2);
    }

    mmio_put(AUX_MU_IER_REG, d);

    return;
}

char asyn_uart_get () {

    char c;

    /* Wait for data */
    while (read_head == read_tail) asm volatile ("nop") ;

    c = read_buffer[read_tail];
    read_tail = (read_tail + 1) & (READ_BUF_SIZE - 1);

    return c;
}

char asyn_uart_getc () {

    char c;
    
    c = asyn_uart_get();
    
    if (c == '\r') 
    {
        c = '\n';
    }

    return c;
}

void asyn_uart_put (char c) {

    write_buffer[write_head] = c;
    write_head = (write_head + 1) & (WRITE_BUF_SIZE - 1);

    /* Enable TX interrupt */
    set_uart_tx_int(true);

    return;
}

void asyn_uart_puth (unsigned int d) {
    unsigned int c;

    for (int i = 28; i >= 0; i -= 4) {
        /* Highest 4 bits */
        c = (d >> i) & 0xF;
        /* Translate to hex */
        c = (c > 9) ? (0x37 + c) : (0x30 + c) ;
        asyn_uart_put(c);
    }

    return;
}

void asyn_uart_putu (unsigned int d) {

    char c;
    bool leading_zero = true;
    unsigned int div = 1000000000;
    unsigned int digit;

    if (d == 0)
    {
        asyn_uart_put('0');
        return;
    }

    while (div)
    {

        digit = d / div;

        if (digit) 
        {
            leading_zero = false;
            d = d - digit * div;
        }

        if (!leading_zero)
        {
            c = '0' + digit;
            asyn_uart_put(c);
        }

        div   = div / 10;

    }

    return;
}

void asyn_uart_puts (char *s) {

    while (*s != '\0') {

        if (*s == '\n') 
        {
            write_buffer[write_head] = '\r';
            write_head = (write_head + 1) & (WRITE_BUF_SIZE - 1);
        }

        write_buffer[write_head] = *s++;
        write_head = (write_head + 1) & (WRITE_BUF_SIZE - 1);
    }

    /* Enable TX interrupt */
    set_uart_tx_int(true);
    
    return;
}