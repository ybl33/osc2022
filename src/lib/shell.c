#include "shell.h"
extern unsigned long __start;
extern unsigned long __end;
extern unsigned long DTB_BASE;
extern unsigned long CPIO_BASE;
extern void from_EL1_to_EL0(unsigned long prog, unsigned long sp);

static unsigned long kernel_start = (unsigned long) &__start;
static unsigned long kernel_end   = (unsigned long) &__end;

void print_system_info () {
    unsigned int board_revision;
    unsigned int board_serial_msb, board_serial_lsb;
    unsigned int mem_base;
    unsigned int mem_size;

    get_board_revision(&board_revision);
    asyn_uart_puts("Board revision   : 0x");
    asyn_uart_puth(board_revision);

    get_board_serial(&board_serial_msb, &board_serial_lsb);
    asyn_uart_puts("\nBoard serial     : 0x");
    asyn_uart_puth(board_serial_msb);
    asyn_uart_puth(board_serial_lsb);

    get_memory_info(&mem_base, &mem_size);
    asyn_uart_puts("\nMem base address : 0x");
    asyn_uart_puth(mem_base);
    asyn_uart_puts("\nMem size         : 0x");
    asyn_uart_puth(mem_size);

    return;
}

void welcome () {
    asyn_uart_puts("   ____   _____  _____    ___   ___ ___  ___     /\\      /\\    __         _ _         __  \n");
    asyn_uart_puts("  / __ \\ / ____|/ ____|  |__ \\ / _ \\__ \\|__ \\   |/\\|    |/\\|  / /        | | |        \\ \\ \n");
    asyn_uart_puts(" | |  | | (___ | |          ) | | | | ) |  ) |      _   _    | |_   _  __| | | ___  ___| |\n");
    asyn_uart_puts(" | |  | |\\___ \\| |         / /| | | |/ /  / /      | | | |   | | | | |/ _` | |/ _ \\/ _ \\ |\n");
    asyn_uart_puts(" | |__| |____) | |____    / /_| |_| / /_ / /_      | |_| |   | | |_| | (_| | |  __/  __/ |\n");
    asyn_uart_puts("  \\____/|_____/ \\_____|  |____|\\___/____|____|      \\__,_|   | |\\__, |\\__,_|_|\\___|\\___| |\n");
    asyn_uart_puts("                                                              \\_\\__/ |                /_/ \n");
    asyn_uart_puts("                                                                |___/                     \n");
    asyn_uart_puts("------------------------------------------------------------------------------------------\n");
    asyn_uart_puts("[Welcom to simple shell, type 'help' for a list of commands]\n");
    return;
}

void help () {
    asyn_uart_puts("----------------Memory Allocator---------------\n");
    asyn_uart_puts("* bmalloc    : buddy system allocation test.\n");
    asyn_uart_puts("* bfree      : buddy system free test.\n");
    asyn_uart_puts("* kmalloc    : slab allocation test.\n");
    asyn_uart_puts("* kfree      : slab free test.\n");
    asyn_uart_puts("------------------File system------------------\n");
    asyn_uart_puts("* cat        : display the file contents.\n");
    asyn_uart_puts("* exec       : load and execute user program.\n");
    asyn_uart_puts("* ls         : list the files in cpio archive.\n");
    asyn_uart_puts("* lsdev      : list the device tree in dtb file.\n");
    asyn_uart_puts("---------------------Other---------------------\n");
    asyn_uart_puts("* clear      : clear the screen.\n");
    asyn_uart_puts("* help       : print help menu.\n");
    asyn_uart_puts("* hello      : print Hello World!\n");
    asyn_uart_puts("* hwinfo     : print hardware infomations.\n");
    asyn_uart_puts("* setTimeout : add a timer task.\n");
    asyn_uart_puts("* reboot     : reboot the device.\n");
    asyn_uart_puts("-----------------------------------------------");
    return;
}

void clear () {
    asyn_uart_put(0x0C);
    return;
}

void buddy_alloc_test (size_t sz) {

    char *mptr;

    /* Allocate memory for string */
    mptr = alloc_pages(sz);

    /* Copy string into allocated memory */
    for (int i = 0; i < sz; i++)
    {
        mptr[i] = 0xFF;
    }

    /* Print prompt */
    asyn_uart_puts("[buddy alloc] Allocated address : 0x");
    asyn_uart_puth((unsigned long)mptr & 0xFFFFFFFF);
    asyn_uart_puts("\n");

    return;
}

void slab_alloc_test (size_t sz) {

    char *mptr;

    /* Allocate memory for string */
    mptr = kmalloc(sz);

    /* Copy string into allocated memory */
    for (int i = 0; i < sz; i++)
    {
        mptr[i] = 0xFF;
    }

    /* Print prompt */
    asyn_uart_puts("[slab alloc] Allocated address : 0x");
    asyn_uart_puth((unsigned long)mptr & 0xFFFFFFFF);
    asyn_uart_puts("\n");

    return;
}

void exec (cpio_header_t *header, char *file_name)
{
    void (*prog)();
    unsigned int current_el;
    char *stack_top;
    
    stack_top = malloc(0x2000);
    stack_top = stack_top + 0x2000;

    prog = cpio_load(header, file_name);

    if (prog == 0)
    {
        asyn_uart_puts("User program not found!\n");
        return;
    }

    // Get current EL
    asm volatile ("mrs %0, CurrentEL" : "=r" (current_el));
    current_el = current_el >> 2;

    // Print prompt
    uart_puts("Current EL: 0x");
    uart_puth(current_el);
    uart_puts("\n");
    uart_puts("User program name: ");
    uart_put('"');
    uart_puts(file_name);
    uart_put('"');
    uart_puts(" (at 0x");
    uart_puth((unsigned long) prog);
    uart_puts(")");
    uart_puts("\n");
    uart_puts("User program stack top: 0x");
    uart_puth((unsigned long) stack_top);
    uart_puts("\n");
    uart_puts("-----------------Entering user program-----------------\n");

    from_EL1_to_EL0((unsigned long)prog, (unsigned long)stack_top);

    return;
}

void setTimeout (char *msg, char *ascii_after) {

    char *data;
    unsigned int i, len, after;

    len = strlen(msg);
    after = atou(ascii_after);

    if (after == -1 )
    {
        return;
    }

    /* Copy message into data buffer */
    data = malloc(len + 2);

    for (i = 0; i < len; i++)
    {
        data[i] = msg[i];
    }

    /* Add a change line */
    data[i]   = '\n';
    data[i+1] = '\0';

    /* Add timer task */
    add_timer(uart_puts, data, after);

    return;
}

//-----------------------------------------------------------------

void put_left () {
    asyn_uart_put(0x1B);
    asyn_uart_put(0x5B);
    asyn_uart_put(0x44);
    return;
}

void put_right () {
    asyn_uart_put(0x1B);
    asyn_uart_put(0x5B);
    asyn_uart_put(0x43);
    return;
}

void echo_back (char c) {

    asyn_uart_put(c);

    if (c == '\n')
    {
        asyn_uart_put('\r');
    }
    else if (c == '\b')
    {
        asyn_uart_put(' ');
        asyn_uart_put('\b');
    }

    return;
}

void read_cmd (char *cmd) {

    char c;
    int pos  = 0;
    int tail = 0;
    int tmp  = 0;

    asyn_uart_puts("> ");

    do {

        c = asyn_uart_getc();
        cmd[tail] = '\0';

        /* Deal with special operation */
        if (c == '\n') 
        {
            echo_back(c);
            break;
        } 
        else if (c == '\b' || c == 127) 
        {
            /* BACKSPACE and DEL */
            if (pos > 0)
            {
                if (pos < tail)
                {
                    strpullout(cmd, pos - 1);
                    tmp = tail - pos;
                    asyn_uart_put('\b');
                    for (int i = 0; i < tmp; i++) asyn_uart_put(cmd[pos - 1 + i]);
                    asyn_uart_put(' ');
                    for (int i = 0; i < tmp + 1; i++) asyn_uart_put('\b');
                }
                else
                {
                    echo_back('\b');
                    cmd[pos - 1] = '\0';
                }
                pos--;
                tail--;
            }
        }
        else if (c == 0x1B)
        {
            /* ESCAPE */
            c = asyn_uart_getc();

            if (c == 0x5B) 
            {
                c = asyn_uart_getc();
                /* Deal with up, down, left, right */
                if (c == 0x41) 
                {
                    /* UP */
                }
                else if (c == 0x42)
                {
                    /* DOWN */
                }
                else if (c == 0x44)
                {
                    /* LEFT */
                    if (pos > 0) 
                    {
                        put_left();
                        pos--;
                    }
                }
                else if (c == 0x43)
                {
                    /* RIGHT */
                    if (pos < tail) 
                    {
                        put_right();
                        pos++;
                    }
                }
                else
                {
                    /* not special operation */
                    asyn_uart_put(0x5B);
                    asyn_uart_put(c);
                }
            }
            else
            {
                asyn_uart_puts("[assert] unknown input.\n");
            }
        }
        else if (c == 0x01)
        {
            /* Go to head */
            for (int i = 0; i < pos; i++) asyn_uart_put('\b');
            pos = 0;
        }
        else if (c == 0x05)
        {
            uart_flush();
            /* Go to tail */
            tmp = tail - pos;
            for (int i = 0; i < tmp; i++) put_right();
            pos = tail;
        }
        else if (c == 0x0B)
        {
            /* Erase till end of line */
            tmp = tail - pos;
            for (int i = 0; i < tmp; i++) asyn_uart_put(' ');
            for (int i = 0; i < tmp; i++) asyn_uart_put('\b');
            cmd[pos] = '\0';
            tail = pos;
        }
        else if (pos >= 0 && pos < CMD_BUF_SIZE - 1)
        {
            if (pos < tail)
            {
                strinsert(cmd, c, pos);
                tmp = tail - pos;
                for (int i = 0; i < tmp + 1; i++) asyn_uart_put(cmd[pos + i]);
                for (int i = 0; i < tmp; i++) asyn_uart_put('\b');
            }
            else
            {
                echo_back(c);
                cmd[pos] = c;
            }
            pos++;
            tail++;
        }

    } while (1);

    return;
}

void do_cmd (char *cmd) {

    char *argv[MAX_ARG_NUM];
    unsigned int argc = 0;
    unsigned int cmd_len = strlen(cmd);
    bool skipping_space = true;

    /* Zero length command */
    if (cmd_len == 0)
    {
        asyn_uart_puts("\n\n");
        return;
    }

    /* Parsing argv */
    for (int i = 0; i < cmd_len; i++) 
    {

        if (cmd[i] != ' ' && skipping_space == 1) 
        {
            argv[argc++] = cmd + i;
            skipping_space = 0;
        } 
        else if (cmd[i] == ' ')
        {
            if (skipping_space == 0) cmd[i] = '\0';
            skipping_space = 1;
        }

    }

    if ( strcmp(argv[0], "clear") == 0 ) 
    {
        clear();
        return;
    }
    else if ( strcmp(argv[0], "help") == 0 ) 
    {
        help();
    }
    else if ( strcmp(argv[0], "hello") == 0 ) 
    {
        asyn_uart_puts("Hello World!");
    }
    else if ( strcmp(argv[0], "hwinfo") == 0 ) 
    {
        print_system_info();
    }
    else if ( strcmp(argv[0], "reboot") == 0 ) 
    {
        reset(1);
    }
    else if ( strcmp(argv[0], "ls") == 0 )
    {
        cpio_ls((cpio_header_t *)CPIO_BASE);
    }
    else if ( strcmp(argv[0], "cat") == 0 )
    {
        if (argc < 2) 
        {
            asyn_uart_puts("Usage: cat <path to file>\n");
        }
        else
        {
            cpio_cat((cpio_header_t *)CPIO_BASE, argv[1]);
        }
    }
    else if ( strcmp(argv[0], "bmalloc") == 0 )
    {
        if (argc < 2) 
        {
            asyn_uart_puts("Usage: bmalloc <size>\n");
        }
        else
        {
            buddy_alloc_test(atou(argv[1]));
        }
    }
    else if ( strcmp(argv[0], "bfree") == 0 )
    {
        if (argc < 2) 
        {
            asyn_uart_puts("Usage: bfree <address in hex>\n");
        }
        else
        {
            unsigned long addr = htoi(argv[1]);
            uart_puts("[bfree test] address: 0x");
            uart_puth(addr);
            uart_puts("\n");
            free_page((void *)addr);
        }
    }
    else if ( strcmp(argv[0], "kmalloc") == 0 )
    {
        if (argc < 2) 
        {
            asyn_uart_puts("Usage: kmalloc <size>\n");
        }
        else
        {
            slab_alloc_test(atou(argv[1]));
        }
    }
    else if ( strcmp(argv[0], "kfree") == 0 )
    {
        if (argc < 2) 
        {
            asyn_uart_puts("Usage: kfree <address in hex>\n");
        }
        else
        {
            unsigned long addr = htoi(argv[1]);
            uart_puts("[kfree test] address: 0x");
            uart_puth(addr);
            uart_puts("\n");
            kfree((void *)addr);
        }
    }
    else if ( strcmp(argv[0], "lsdev") == 0 )
    {
        fdt_traverse((struct fdt_header *)DTB_BASE, lsdev_callback);
    }
    else if ( strcmp(argv[0], "exec") == 0 )
    {
        if (argc < 2) 
        {
            asyn_uart_puts("Usage: exec <file name>\n");
        }
        else
        {
            exec((cpio_header_t *)CPIO_BASE, argv[1]);
        }
    }
    else if ( strcmp(argv[0], "setTimeout") == 0 )
    {
        if (argc < 3) 
        {
            asyn_uart_puts("Usage: setTimeout <msg> <after>\n");
        }
        else
        {
            setTimeout(argv[1], argv[2]);
        }
    }
    else 
    {
        asyn_uart_puts("invalid command.");
    }

    asyn_uart_puts("\n\n");

    return;
}

void shell_start () {

    char cmd[CMD_BUF_SIZE];

    // System init
    asyn_uart_init();
    cpio_init();

    // Reserve memory
    unsigned int dtb_size = SWAP_UINT32(((struct fdt_header *)DTB_BASE)->totalsize);
    memory_reserve(0x0000, 0x1000);                // Spin tables for multicore boot
    memory_reserve(kernel_start, kernel_end);      // Kernel image
    memory_reserve(CPIO_BASE, CPIO_BASE + 8192);   // initramfs 8KB
    memory_reserve(DTB_BASE, DTB_BASE + dtb_size); // dtb 8KB

    // Print prompt
    clear();
    welcome();

    while (1) {
        read_cmd(cmd);
        do_cmd(cmd);
    }

}