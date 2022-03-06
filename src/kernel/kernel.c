#include "shell.h"
#include "uart.h"

unsigned long DTB_BASE;

int main() {

    register unsigned long x0 asm ("x0");
    DTB_BASE = x0;

    uart_init();
    shell_start();

    return 0;
}