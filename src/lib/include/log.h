#ifndef __LOG__H__
#define __LOG__H__
#include "stddef.h"
#include "uart.h"

void log_put(char c, bool on);
void log_puth(unsigned int d, bool on);
void log_putu(unsigned int d, bool on);
void log_puts(char *s, bool on);

#endif