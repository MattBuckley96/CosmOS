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
static struct Dir dir;

static char* builtin_commands[] = {
    "clear",
    "exit",
    "echo",
    "mkfs",
    "ls",
    "stat",
    "cat",
    "cd",
};

static void (*builtin_table[])(int argc, char** argv) = {
    shell_clear,
    shell_exit,
    shell_echo,
    shell_mkfs,
    shell_ls,
    shell_stat,
    shell_cat,
    shell_cd,
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

void shell_mkfs(int argc, char** argv)
{
    int err = fs_create();
    if (err)
    {
        printf("mkfs: error creating file system!\n");
        return;
    }

    printf("%s: successfully created file system!\n", argv[0]);
}

void shell_ls(int argc, char** argv)
{
    dir_list(&dir);
}

void shell_stat(int argc, char** argv)
{
    struct File file;

    int err = file_open(&file, &dir, argv[1]);
    if (err)
    {
        printf("stat: %s: file not found!\n", argv[1]);
        return;
    }

    struct Inode inode;
    get_inode(file.inode, &inode);

    printf("inode: %i\n", file.inode);
    printf("size: %i\n", inode.size);
    printf("type: ");

    switch (inode.type)
    {
    case (FS_FILE):
        printf("file\n");
        break;

    case (FS_DIR):
        printf("directory\n");
        break;
    }

    u32 blocks = block_count(&inode);
    printf("blocks: %i\n", blocks);
}

void shell_cat(int argc, char** argv)
{
    struct File file;

    int err = file_open(&file, &dir, argv[1]);
    if (err)
    {
        printf("cat: %s: file not found!\n", argv[1]);
        return;
    }

    struct Inode inode;
    err = get_inode(file.inode, &inode);
    if (err)
    {
        printf("cat: couldn't load inode!\n");
        return;
    }

    if (inode.type == FS_DIR)
    {
        printf("cat: %s: is a directory.\n", argv[1]);
        return;
    }

    // NOTE: hardcoded for now;
    char buf[8192] = {};

    err = file_read(&file, buf);
    if (err)
    {
        printf("%s told me to tell you it couldnt read...", argv[1]);
        return;
    }

    printf("%s\n", buf);
}

void shell_cd(int argc, char** argv)
{
    if (argc < 2)
    {
        dir.inode = 1;
        return;
    }

    int err = dir_open(&dir, argv[1]);
    if (err)
        printf("%s: %s: couldnt find directory!\n", argv[0], argv[1]);
}

///////////////////////////////////////////////

void shell_main(void)
{
    running = true;

    dir = (struct Dir) {
        .inode = 1
    };

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
