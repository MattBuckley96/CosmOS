#include "types.h"
#include "shell.h"
#include "printf.h"
#include "string.h"
#include "drivers/vga.h"
#include "drivers/keyboard.h"
#include "cpu/io.h"
#include "fs/fs.h"

///////////////////////////////////////////////

#define BUF_SIZE 256
#define MAX_ARGS 8

///////////////////////////////////////////////

static bool running;
static char args[MAX_ARGS][BUF_SIZE];

static char* builtin_commands[] = {
    "clear",
    "exit",
    "echo",
    "mkfs",
    "ls",
};

static void (*builtin_table[])(int argc, char** argv) = {
    shell_clear,
    shell_exit,
    shell_echo,
    shell_mkfs,
    shell_ls,
};

///////////////////////////////////////////////

static void prompt(void)
{
    vga_puts("Cosm", VGA_LIGHT_MAGENTA);
    vga_puts("OS", VGA_LIGHT_YELLOW);
    vga_puts(":", VGA_LIGHT_CYAN);
    vga_puts("$ ", VGA_WHITE);
}

static void get_line(char* line)
{
    int i = 0;
    while (i < BUF_SIZE)
    {
        char c = keyboard_getc();

        if (!c)
        {
            continue;
        }

        switch (c)
        {
        case '\n':
            printf("\n");
            return;

        case '\b':
            if (i <= 0) 
            {
                break;
            }

            line[--i] = '\0';
            printf("\b \b");
            break;

        default:
            printf("%c", c);
            line[i++] = c;
            break;
        }
    }

    line[i] = '\0';
}

static void split_line(const char* line, int* argc, char** argv)
{
    for (int i = 0; i < MAX_ARGS; i++)
    {
        argv[i] = NULL;
        for (int j = 0; j < BUF_SIZE; j++)
        {
            args[i][j] = '\0';
        }
    }

    *argc = 0;

    while (*line && *argc < MAX_ARGS)
    {
        while (*line == ' ')
        {
            line++;
        }

        if (*line == '\0')
        {
            break;
        }

        int i = 0;
        while (*line != ' ' && *line && i < BUF_SIZE - 1)
        {
            args[*argc][i++] = *line++;
        }
        args[*argc][i] = '\0';

        argv[*argc] = args[*argc];
        (*argc)++;
    }
}

static void execute(int argc, char** argv)
{
    if (argc <= 0)
    {
        return;
    }

    int size = sizeof(builtin_commands) / sizeof(char*);

    for (int i = 0; i < size; i++)
    {
        if (strcmp(argv[0], builtin_commands[i]) == 0)
        {
            (*builtin_table[i])(argc, argv);
            return;
        }
    }

    printf("%s: command not found!\n", argv[0]);
}

void shell_clear(int argc, char** argv)
{
    vga_clear();
}

void shell_exit(int argc, char** argv)
{
    running = false;

    // HACK: shutdown qemu
    outw(0x604, 0x2000);
}

void shell_echo(int argc, char** argv)
{
    for (int i = 1; i < argc; i++)
    {
        printf("%s ", argv[i]);
    }
    printf("%c", '\n');
}

void shell_ls(int argc, char** argv)
{
    fs_list();
}

///////////////////////////////////////////////

void shell_main(void)
{
    running = true;

    while (running)
    {
        char line[BUF_SIZE] = {};
        int argc = 0;
        char* argv[MAX_ARGS];

        prompt();
    
        get_line(line);
        split_line(line, &argc, argv);
        execute(argc, argv);
    }
}
