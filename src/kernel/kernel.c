#include "shell.h"
#include "uart.h"

int main() {

    uart_init();
    shell_start();

    return 0;
}