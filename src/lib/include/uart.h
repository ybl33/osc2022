#ifndef __UART__H__
#define __UART__H__
#include "stddef.h"
#include "gpio.h"
#include "aux.h"
#include "exception.h"
#define AUX_MU_LSR_TRANS_EMPTY (1 << 5)
#define AUX_MU_LSR_DATA_READY  (1)

void uart_init();
bool uart_rx_valid();
bool uart_tx_ready();
void uart_flush();
char uart_get();
char uart_getc();
void uart_put(char c);
void uart_puth(unsigned int d);
void uart_putu(unsigned int d);
void uart_puts(char *s);

/* Asynchronous Read and Write */
#define READ_BUF_SIZE          (1024)
#define WRITE_BUF_SIZE         (1024)
extern char read_buffer[READ_BUF_SIZE];
extern char write_buffer[WRITE_BUF_SIZE];
// TO DO : maybe need to handle buffer overflow

extern unsigned int read_head;
extern unsigned int write_head;
extern unsigned int read_tail;
extern unsigned int write_tail;

void asyn_uart_init();
void set_uart_rx_int(bool enable);
void set_uart_tx_int(bool enable);
char asyn_uart_get();
char asyn_uart_getc();
void asyn_uart_put(char c);
void asyn_uart_puth(unsigned int d);
void asyn_uart_putu(unsigned int d);
void asyn_uart_puts(char *s);

#endif