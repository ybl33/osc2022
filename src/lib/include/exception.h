#ifndef __EXCEPTION__H__
#define __EXCEPTION__H__
#include "stddef.h"
#include "heap.h"
#include "mmio.h"
#include "uart.h"
#include "timer.h"

#define CORE_0_TIMER_INT_CTL (0x40000040)
#define CORE_0_IRQ_SOURCE    (0x40000060)
#define CORE_IRQ_CNTPNS      (1 << 1)
/* Interrupt Registers */
#define INT_BASE             (MMIO_BASE + 0x0000B000)
#define IRQ_BASIC_PENDING    (INT_BASE + 0x200)
#define IRQ_1_PENDING        (INT_BASE + 0x204)
#define IRQ_2_PENDING        (INT_BASE + 0x208)
#define IRQs_1_ENABLE        (INT_BASE + 0x210)
#define IRQs_2_ENABLE        (INT_BASE + 0x214)
#define IRQs_BASIC_ENABLE    (INT_BASE + 0x214)
#define IRQs_1_DISABLE       (INT_BASE + 0x21C)
#define IRQs_2_DISABLE       (INT_BASE + 0x220)
#define IRQs_BASIC_DISABLE   (INT_BASE + 0x224)
#define IRQ_1_SYS_TIMER      (1 << 1)
#define IRQs_1_HAS_PENDING   (1 << 8)
#define IRQ_1_LOCAL_TIMER    (1 << 11)
#define IRQ_1_AUX_INT        (1 << 29)

/* PRIO */
#define TIMER_IRQ_PRIO       (0)
#define UART_IRQ_PRIO        (1)

struct exception_task {

    unsigned int priority;
    void (*handler) ();
    struct exception_task *next_task;

};

extern struct exception_task *exception_task_list;

void set_interrupt(bool enable);
void set_timer_interrupt(bool enable);
void set_aux_int(bool enable);
void syn_handler();
void irq_handler();
void undefined_handler();
void uart_irq_handler();
void timer_irq_handler();

#endif