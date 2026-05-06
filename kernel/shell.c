#include <stdarg.h>
#include "shell.h"
#include "types.h"
#include "keyboard.h"
#include "util.h"
#include "vga.h"
#include "io.h"

#define BUF_SIZE 512
#define MAX_ARGS 8

char arg_buf[BUF_SIZE][MAX_ARGS];

void print_prompt(void)
{
    vga_set_color(VGA_LIGHT_MAGENTA, VGA_BLACK);
    kprintf("Cosm");

    vga_set_color(VGA_LIGHT_YELLOW, VGA_BLACK);
    kprintf("OS");

    vga_set_color(VGA_WHITE, VGA_BLACK);
    kprintf(":");

    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    kprintf("/");

    vga_set_color(VGA_WHITE, VGA_BLACK);
    kprintf("$ ");
    vga_set_color(VGA_GRAY, VGA_BLACK);
}

void get_line(char* line)
{
    u32 i = 0;

    while (i < BUF_SIZE) {
        u8 c = keyboard_getchar();

        switch (c) {
        case 0:
            continue;
        
        case '\b':
            if (i > 0) {
                kprintf("%c", c);
                line[--i] = '\0';
            }
            break;

        case '\n':
            kprintf("%c", c);
            return;

        default:
            kprintf("%c", c);
            line[i++] = c;
            break;
        }
    }
}

void parse_line(char* line, int* argc, char** argv)
{
    memset(arg_buf, 0, sizeof(arg_buf));

    *argc = 0;

    while (*line && *argc < MAX_ARGS) {
        // skip whitespace
        while (*line == ' ') {
            line++;
        }

        if (*line == '\0') {
            break;
        }

        int i = 0;
        while (*line != ' ' && *line && i < BUF_SIZE - 1) {
            arg_buf[*argc][i++] = *line++;
        }
        arg_buf[*argc][i] = '\0';

        argv[*argc] = arg_buf[*argc];
        (*argc)++;
    }
}

void exec_cmd(int argc, char** argv)
{
    if (argc < 1) {
        return;
    }

    if (strcmp(argv[0], "clear") == 0) {
        vga_clear();
        return;
    }

    if (strcmp(argv[0], "exit") == 0) {
        // qemu shutdown
        outw(0x604, 0x2000);
        return;
    }

    if (strcmp(argv[0], "regs") == 0) {
        print_regs();
        return;
    }

    if (strcmp(argv[0], "echo") == 0) {
        for (int i = 1; i < argc; i++) {
            kprintf("%s ", argv[i]);
        } 
        kprintf("\n");
        return;
    }

    kprintf("%s: command not found!\n", argv[0]);
}

void shell(void)
{
    for (;;) {
        char line[BUF_SIZE];
        int argc = 0;
        char* argv[MAX_ARGS];

        memset(line, 0, sizeof(line));
        memset(argv, 0, sizeof(argv));

        print_prompt();
        get_line(line);
        parse_line(line, &argc, argv);
        exec_cmd(argc, argv);
    }
}