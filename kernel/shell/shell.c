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
static struct File dir;

static char* builtin_commands[] = {
    "clear",
    "exit",
    "echo",
    "mkfs",
    "fsinfo",
    "ls",
    "stat",

    "help",
};

static void (*builtin_table[])(int argc, char** argv) = {
    shell_clear,
    shell_exit,
    shell_echo,
    shell_mkfs,
    shell_fsinfo,
    shell_ls,
    shell_stat,

    shell_help,
};

#define BUILTIN_SIZE (sizeof(builtin_commands) / sizeof(char*))

///////////////////////////////////////////////

static void prompt(void)
{
    vga_puts("Cosm", VGA_LIGHT_MAGENTA);
    vga_puts("OS", VGA_LIGHT_YELLOW);
    vga_puts(":", VGA_WHITE);
    vga_puts("/", VGA_LIGHT_CYAN);

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

    for (int i = 0; i < BUILTIN_SIZE; i++)
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
    char buf[512];
    int pos = 0;

    for (int i = 1; i < argc; i++)
    {
        while (*argv[i])
        {
            buf[pos++] = *argv[i]++;
        }
        buf[pos++] = ' ';
    }

    buf[pos] = '\0';
    printf("%s\n", buf);
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

void shell_fsinfo(int argc, char** argv)
{
    printf("Superblock:\n");    
    printf("magic: %i\n", sb.magic);
    printf("inodes: %i\n", sb.inodes);
    printf("free_inodes: %i\n", sb.free_inodes);
    printf("blocks: %i\n", sb.blocks);
    printf("free_blocks: %i\n", sb.free_blocks);
    printf("block_size: %i\n", sb.block_size);
    printf("state: %i\n", sb.state);

    printf("\nDescriptor:\n");
    printf("inode_bitmap_addr: %i\n", desc.inode_bitmap_addr);
    printf("block_bitmap_addr: %i\n", desc.block_bitmap_addr);
    printf("inodes_addr: %i\n", desc.inodes_addr);
    printf("blocks_addr: %i\n", desc.blocks_addr);
}

void shell_ls(int argc, char** argv)
{
    dir_list(&dir);
}

void shell_stat(int argc, char** argv)
{
    struct Inode inode;

    if (argc < 2)
    {
        printf("%s: usage: stat <inode>\n", argv[0]);
        return;
    }

    u32 i = *argv[1] - '0';

    if (i == 0)
    {
        printf("%s: invalid inode: %i\n", argv[0], i);
        return;
    }

    int err = inode_get(i, &inode);
    if (err)
        return;

    printf("inode: %i\n", i);
    printf("size: %i\n", inode.size);
    printf("blocks: %i\n", inode_block_count(&inode));

    printf("type: ");

    switch (inode.type)
    {
        case FS_FILE:
        {
            printf("file\n");
        } break;

        case FS_DIR:
        {
            printf("dir\n");
        } break;
    }
}

void shell_help(int argc, char** argv)
{
    printf("Available commands (%i):\n\n", BUILTIN_SIZE);

    for (int i = 0; i < BUILTIN_SIZE; i++)
    {
        printf("%s\n", builtin_commands[i]);
    }
}

///////////////////////////////////////////////

void shell_main(void)
{
    running = true;

    dir = (struct File){
        .inode = 1,
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
