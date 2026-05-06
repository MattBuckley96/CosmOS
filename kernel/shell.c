#include <stdarg.h>
#include "shell.h"
#include "types.h"
#include "keyboard.h"
#include "util.h"
#include "vga.h"
#include "io.h"

#define BUF_SIZE 512

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

void exec_line(char* line)
{
    if (strcmp(line, "clear") == 0) {
        vga_clear();
        return;
    }

    if (strcmp(line, "exit") == 0) {
        // qemu shutdown
        outw(0x604, 0x2000);
        return;
    }
}

void shell(void)
{
    for (;;) {
        char line[BUF_SIZE];
        memset(line, 0, BUF_SIZE);

        print_prompt();
        get_line(line);
        exec_line(line);
    }
}