#ifndef __UART__H__
#define __UART__H__
#include "stddef.h"
#include "gpio.h"
#include "aux.h"
#define AUX_MU_LSR_TRANS_EMPTY (1 << 5)
#define AUX_MU_LSR_DATA_READY  (1)

void uart_init();
void uart_flush();
char uart_get();
char uart_getc();
void uart_putc(char c);
void uart_puth(unsigned int d);
void uart_puts(char *s);
#endif