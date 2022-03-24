#ifndef __EXCEPTION__H__
#define __EXCEPTION__H__
#include "uart.h"
#include "utils.h"

void syn_handler();
void irq_handler();
void undefined_handler();

#endif