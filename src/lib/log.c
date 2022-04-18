#include "log.h"

void log_put(char c, bool on) {
    
    if (on)
    {
        uart_put(c);
    }

    return;
}

void log_puth(unsigned int d, bool on) {
    
    if (on)
    {
        uart_puth(d);
    }

    return;
}

void log_putu(unsigned int d, bool on) {
    
    if (on)
    {
        uart_putu(d);
    }

    return;
}

void log_puts(char *s, bool on) {
    
    if (on)
    {
        uart_puts(s);
    }

    return;
}
