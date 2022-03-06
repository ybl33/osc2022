#ifndef __SHELL__H__
#define __SHELL__H__
#include "cpio.h"
#include "utils.h"
#include "uart.h"
#include "string.h"
#include "heap.h"
#include "dtb.h"
#define CMD_BUF_SIZE 32
#define MAX_ARG_NUM  4
void print_system_info();
void welcome();
void help();
void clear();
void salloc(char *s);
void mdump(char *s1, char *s2);
void put_left();
void put_right();
void echo_back(char c);
void read_cmd(char *cmd);
void do_cmd(char *cmd);
void shell_start();
#endif