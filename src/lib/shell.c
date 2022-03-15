#include "shell.h"

extern unsigned long DTB_BASE;
extern unsigned long CPIO_BASE;

void print_system_info () {
    unsigned int board_revision;
    unsigned int board_serial_msb, board_serial_lsb;
    unsigned int mem_base;
    unsigned int mem_size;

    get_board_revision(&board_revision);
    uart_puts("Board revision   : 0x");
    uart_puth(board_revision);

    get_board_serial(&board_serial_msb, &board_serial_lsb);
    uart_puts("\nBoard serial     : 0x");
    uart_puth(board_serial_msb);
    uart_puth(board_serial_lsb);

    get_memory_info(&mem_base, &mem_size);
    uart_puts("\nMem base address : 0x");
    uart_puth(mem_base);
    uart_puts("\nMem size         : 0x");
    uart_puth(mem_size);

    return;
}

void welcome () {
    uart_puts("   ____   _____  _____    ___   ___ ___  ___     /\\      /\\    __         _ _         __  \n");
    uart_puts("  / __ \\ / ____|/ ____|  |__ \\ / _ \\__ \\|__ \\   |/\\|    |/\\|  / /        | | |        \\ \\ \n");
    uart_puts(" | |  | | (___ | |          ) | | | | ) |  ) |      _   _    | |_   _  __| | | ___  ___| |\n");
    uart_puts(" | |  | |\\___ \\| |         / /| | | |/ /  / /      | | | |   | | | | |/ _` | |/ _ \\/ _ \\ |\n");
    uart_puts(" | |__| |____) | |____    / /_| |_| / /_ / /_      | |_| |   | | |_| | (_| | |  __/  __/ |\n");
    uart_puts("  \\____/|_____/ \\_____|  |____|\\___/____|____|      \\__,_|   | |\\__, |\\__,_|_|\\___|\\___| |\n");
    uart_puts("                                                              \\_\\__/ |                /_/ \n");
    uart_puts("                                                                |___/                     \n");
    uart_puts("------------------------------------------------------------------------------------------\n");
    uart_puts("[Welcom to simple shell, type 'help' for a list of commands]\n");
    return;
}

void help () {
    uart_puts("----------------Memory Allocator---------------\n");
    uart_puts("* salloc : allocate memory for the string.\n");
    uart_puts("* mdump  : dump content of memory.\n");
    uart_puts("------------------File system------------------\n");
    uart_puts("* cat     : display the file contents.\n");
    uart_puts("* ls      : list the files in cpio archive.\n");
    uart_puts("* lsdev   : list the device tree in dtb file.\n");
    uart_puts("---------------------Other---------------------\n");
    uart_puts("* clear   : clear the screen.\n");
    uart_puts("* help    : print help menu.\n");
    uart_puts("* hello   : print Hello World!\n");
    uart_puts("* sysinfo : print system infomations.\n");
    uart_puts("* reboot  : reboot the device.\n");
    uart_puts("-----------------------------------------------");
    return;
}

void clear () {
    uart_putc(0x0C);
    return;
}

void salloc (char *s) {

    size_t sz;
    char *mptr;

    /* Allocate memory for string */
    sz = strlen(s);
    mptr = malloc(sz);

    /* Copy string into allocated memory */
    strcpy(mptr, s);

    /* Print prompt */
    uart_puts("Allocated address : 0x");
    uart_puth((unsigned long)mptr & 0xFFFFFFFF);
    uart_puts("\n");
    uart_puts("Allocated size    : 0x");
    uart_puth(sz);

    return;
}

void mdump (char *s1, char *s2) {

    unsigned long addr;
    unsigned long len;
    char c;

    if (s1[0] != '0' || s1[1] != 'x' || s2[0] != '0' || s2[1] != 'x')
    {
        uart_puts("Wrong input, address should start with 0x\n");
    }
    else
    {
        s1   = s1 + 2;
        s2   = s2 + 2;
        addr = htoin(s1, strlen(s1));
        len  = htoin(s2, strlen(s2));
        len  = (len + 3) >> 2;
        uart_puts("  Address    Content (Hex)    ASCII\n");
        for (int i = 0; i < len; i++)
        {
            uart_puts("0x");
            uart_puth(addr);
            uart_puts("    0x");
            uart_puth(*(unsigned int *)addr);
            uart_puts("     ");

            for (int j = 0; j < 4; j++) {

                c = ((char *)addr)[3 - j];

                if (c >= 32 && c <= 126)
                {
                    uart_putc(c);
                }
                else
                {
                    uart_putc(' ');
                }

                uart_putc(' ');

            }

            uart_puts("\n");
            addr += 4;
        }
    }

    return;
}

//-----------------------------------------------------------------

void put_left () {
    uart_putc(0x1B);
    uart_putc(0x5B);
    uart_putc(0x44);
    return;
}

void put_right () {
    uart_putc(0x1B);
    uart_putc(0x5B);
    uart_putc(0x43);
    return;
}

void echo_back (char c) {

    uart_putc(c);

    if (c == '\n')
    {
        uart_putc('\r');
    }
    else if (c == '\b')
    {
        uart_putc(' ');
        uart_putc('\b');
    }

    return;
}

void read_cmd (char *cmd) {

    char c;
    int pos  = 0;
    int tail = 0;
    int tmp  = 0;

    uart_puts("> ");

    do {

        c = uart_getc();
        cmd[tail] = '\0';
        /* Deal with special operation */
        if (c == '\n') 
        {

            // Debug
            // uart_puts("\n  ");
            // uart_puts(cmd);
            // uart_puts("\n  pos: ");
            // uart_puth(pos);
            // uart_puts("\n  tail: ");
            // uart_puth(tail);

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
                    uart_putc('\b');
                    for (int i = 0; i < tmp; i++) uart_putc(cmd[pos - 1 + i]);
                    uart_putc(' ');
                    for (int i = 0; i < tmp + 1; i++) uart_putc('\b');
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
            c = uart_getc();

            if (c == 0x5B) 
            {
                c = uart_getc();
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
                    uart_putc(0x5B);
                    uart_putc(c);
                }
            }
            else
            {
                uart_puts("[assert] unknown input.\n");
            }
        }
        else if (c == 0x01)
        {
            /* Go to head */
            for (int i = 0; i < pos; i++) uart_putc('\b');
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
            for (int i = 0; i < tmp; i++) uart_putc(' ');
            for (int i = 0; i < tmp; i++) uart_putc('\b');
            cmd[pos] = '\0';
            tail = pos;
        }
        else if (pos >= 0 && pos < CMD_BUF_SIZE - 1)
        {
            if (pos < tail)
            {
                strinsert(cmd, c, pos);
                tmp = tail - pos;
                for (int i = 0; i < tmp + 1; i++) uart_putc(cmd[pos + i]);
                for (int i = 0; i < tmp; i++) uart_putc('\b');
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
        uart_puts("Hello World!");
    }
    else if ( strcmp(argv[0], "sysinfo") == 0 ) 
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
            uart_puts("Usage: cat <path to file>\n");
        }
        else
        {
            cpio_cat((cpio_header_t *)CPIO_BASE, argv[1]);
        }
    }
    else if ( strcmp(argv[0], "salloc") == 0 )
    {
        if (argc < 2) 
        {
            uart_puts("Usage: salloc <string>\n");
        }
        else
        {
            salloc(argv[1]);
        }
    }
    else if ( strcmp(argv[0], "mdump") == 0 )
    {
        if (argc < 3) 
        {
            uart_puts("Usage: mdump <address> <length>\n");
        }
        else
        {
            mdump(argv[1], argv[2]);
        }
    }
    else if ( strcmp(argv[0], "lsdev") == 0 )
    {
        fdt_traverse((struct fdt_header *)DTB_BASE, lsdev_callback);
    }
    else 
    {
        uart_puts("invalid command.");
    }

    uart_puts("\n\n");

    return;
}

void shell_start () {

    char cmd[CMD_BUF_SIZE];

    cpio_init();
    clear();
    welcome();

    while (1) {
        read_cmd(cmd);
        do_cmd(cmd);
    }

}