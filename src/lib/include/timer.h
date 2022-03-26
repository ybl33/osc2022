#ifndef __TIMER__H__
#define __TIMER__H__
#include "stddef.h"
#include "uart.h"
#include "heap.h"

struct timer_task {

    unsigned int execute_time;
    void (*callback) ();
    void *data;
    struct timer_task *next_task;

};

extern struct timer_task *timer_task_list;

void add_timer(void (*callback) (), void *data, unsigned int after);
void set_timeout(unsigned int seconds);
unsigned long time();
void print_time();

#endif