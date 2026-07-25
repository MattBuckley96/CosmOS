#include "shell.h"
#include "types.h"
#include "util.h"
#include "console.h"
#include "memory.h"
#include "cpu/io.h"
#include "drivers/keyboard.h"
#include "drivers/ata.h"
#include "fs/fs.h"

#define BUF_SIZE 512
#define MAX_ARGS 8

u8 arg_buf[MAX_ARGS][BUF_SIZE];
bool running;

// start at root
u32 cwd = 1;

void print_prompt(void) {
    kprintf("%F[%FCosm%FOS%F:%F/%F]# %F",
        VGA_WHITE,
        VGA_LIGHT_MAGENTA,
        VGA_LIGHT_YELLOW,
        VGA_WHITE,
        VGA_LIGHT_CYAN,
        VGA_WHITE,
        VGA_GRAY);
}

void get_line(u8* line) {
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

        case '\t':
            for (u32 j = 0; j < 4; j++) {
                kprintf(" ");
                line[i++] = ' ';
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

void parse_line(u8* line, u32* argc, u8** argv) {
    mem_zero(arg_buf, sizeof(arg_buf));

    *argc = 0;

    while (*line && *argc < MAX_ARGS) {
        // skip whitespace
        while (*line == ' ') {
            line++;
        }

        if (*line == '\0') {
            break;
        }

        u32 i = 0;
        while (*line != ' ' && *line && i < BUF_SIZE - 1) {
            arg_buf[*argc][i++] = *line++;
        }
        arg_buf[*argc][i] = '\0';

        argv[*argc] = arg_buf[*argc];
        (*argc)++;
    }
}

void exec_cmd(u32 argc, u8** argv) {
    if (argc < 1) {
        return;
    }

    if (strcmp(argv[0], "clear") == 0) {
        console_clear();
        return;
    }

    if (strcmp(argv[0], "shutdown") == 0) {
        // qemu shutdown
        outw(0x604, 0x2000);
        return;
    }

    if (strcmp(argv[0], "exit") == 0) {
        // qemu shutdown
        running = false;
        return;
    }

    if (strcmp(argv[0], "echo") == 0) {
        u8 buf[512];
        u32 pos = 0;
        mem_zero(buf, sizeof(buf));

        for (u32 i = 1; i < argc; i++) {
            if (strcmp(argv[i], ">>") == 0) {
                buf[pos] = '\0';
                i++; // next arg
                if (i >= argc) {
                    kprintf("%s: please enter a valid file name\n", argv[0]);
                    return;
                }

                u8* file_name = argv[i];

                u32 id = fs_dir_find(cwd, file_name);
                if (id == 0) {
                    id = fs_create_file(cwd, file_name);
                    if (id == 0) {
                        kprintf("%s: couldnt open file: %s\n", argv[0], file_name);
                        return;
                    }
                }

                inode_t inode;
                fs_get_inode(id, &inode);
                if (inode.type == FS_DIR) {
                    kprintf("%s: can't write to a directory!\n", argv[0]);
                    return;
                }

                // (pos - 1) to not write extra space
                u32 written = inode_write(id, buf, pos - 1);
                if (written == 0) {
                    kprintf("%s: failed to write to file: %s\n", argv[0], file_name);
                }

                return;
            }

            while (*argv[i]) {
                buf[pos] = *argv[i]++;
                pos++;
            }
            buf[pos++] = ' ';
        } 
        kprintf("%s\n", buf);
        return;
    }

    if (strcmp(argv[0], "memory") == 0) {
        kprintf("%F[%Fmemory%F]%F\n",
            VGA_WHITE, VGA_LIGHT_GREEN, VGA_WHITE, VGA_GRAY);

        u32 used = get_used_memory();
        u32 total = get_total_memory();
        u32 free = total - used;

        kprintf("total free: %u (%u MiB)\n", free, (free / 1024 / 1024));
        kprintf("total used: %u\n", used);
        return;
    }

    if (strcmp(argv[0], "start") == 0) {
        // HACK: it works, but wtf am i thinking
        typedef int (*entry_t)(void);

        void* addr = malloc(512);
        ata_read_sectors(42, addr, 1);

        entry_t entry = (entry_t)addr;
        entry();
        return;
    }

    if (strcmp(argv[0], "mkfs") == 0) {
        fs_create();
        return;;
    }

    if (strcmp(argv[0], "fsinfo") == 0) {
        fs_print();
        return;;
    }

    if (strcmp(argv[0], "stat") == 0) {
        if (argc < 2) {
            kprintf("%s: usage: %s <file-name>\n", argv[0], argv[0]);
            return;
        }

        u32 id = fs_dir_find(cwd, argv[1]);
        if (id == 0) {
            kprintf("%s: couldn't open file: %s!\n", argv[0], argv[1]);
            return;
        }

        inode_t inode;
        fs_get_inode(id, &inode);

        kprintf("inode: %u\n", id);

        kprintf("type: ");
        if (inode.type == FS_FILE) {
            kprintf("file\n");
        }
        if (inode.type == FS_DIR) {
            kprintf("dir\n");
        }

        kprintf("size: %u\n", inode.size);
        kprintf("blocks: %u\n", inode_get_block_count(id));
        return;
    }

    if (strcmp(argv[0], "read") == 0) {
        u32 id = fs_dir_find(cwd, argv[1]);
        if (id == 0) {
            kprintf("%s: couldn't open file: %s!\n", argv[0], argv[1]);
            return;
        }

        inode_t inode;
        fs_get_inode(id, &inode);

        u8 buf[inode.size];
        u32 read = inode_read(id, buf, inode.size);
        buf[read] = '\0';

        kprintf("%s\n", buf);

        kprintf("bytes read: %u\n", read);
        return;
    }

    if (strcmp(argv[0], "ls") == 0) {
        u32 id = cwd;
        if (argc > 1) {
            id = fs_dir_find(cwd, argv[1]);
            if (id == 0) {
                kprintf("%s: couldn't open file: %s!\n", argv[0], argv[1]);
                return;
            }
        }
        fs_list_dir(id);
        return;
    }

    if (strcmp(argv[0], "mkdir") == 0) {
        if (argc < 2) {
            kprintf("%s: usage: %s <file-name>\n", argv[0], argv[0]);
            return;
        }

        u32 id = fs_dir_find(cwd, argv[1]);
        if (id > 0) {
            kprintf("%s: the town ain't big enough for the two of %s\n", argv[0], argv[1]);
            return;
        }

        id = fs_create_dir(cwd, argv[1]);
        if (id == 0) {
            kprintf("%s: failed to create directory!\n", argv[0]);
            return;
        }
        return;
    }

    if (strcmp(argv[0], "touch") == 0) {
        if (argc < 2) {
            kprintf("%s: usage: %s <file-name>\n", argv[0], argv[0]);
            return;
        }

        u32 id = fs_dir_find(cwd, argv[1]);
        if (id > 0) {
            kprintf("%s: the town ain't big enough for the two of %s\n", argv[0], argv[1]);
            return;
        }

        id = fs_create_file(cwd, argv[1]);
        if (id == 0) {
            kprintf("%s: failed to create file!\n", argv[0]);
            return;
        }
        return;
    }

    if (strcmp(argv[0], "rm") == 0) {
        if (argc < 2) {
            kprintf("%s: usage: %s <file-name>\n", argv[0], argv[0]);
            return;
        }

        u32 id = fs_dir_find(cwd, argv[1]);
        if (id == 0) {
            kprintf("%s: couldn't find file: %s\n", argv[0], argv[1]);
            return;
        }

        inode_t inode;
        fs_get_inode(id, &inode);
        if (inode.type == FS_DIR) {
            kprintf("%s: %s is a directory, use rmdir\n", argv[0], argv[1]);
            return;
        }

        fs_delete_file(cwd, id);
        return;
    }

    if (strcmp(argv[0], "rmdir") == 0) {
        if (argc < 2) {
            kprintf("%s: usage: %s <file-name>\n", argv[0], argv[0]);
            return;
        }

        u32 id = fs_dir_find(cwd, argv[1]);
        if (id == 0) {
            kprintf("%s: couldn't find file: %s\n", argv[0], argv[1]);
            return;
        }

        inode_t inode;
        fs_get_inode(id, &inode);
        if (inode.type == FS_FILE) {
            kprintf("%s: %s is a file, use rm\n", argv[0], argv[1]);
            return;
        }

        fs_delete_dir(id);
        return;
    }

    if (strcmp(argv[0], "cd") == 0) {
        if (argc < 2) {
            return;
        }

        u32 id = fs_dir_find(cwd, argv[1]);
        if (id == 0) {
            kprintf("%s: directory doesn't exist\n!", argv[0]);
            return;
        }

        inode_t inode;
        fs_get_inode(id, &inode);

        if (inode.type != FS_DIR) {
            kprintf("%s: %s is not a directory\n", argv[0], argv[1]);
            return;
        }

        cwd = id;
        return;
    }

    kprintf("%s: command not found!\n", argv[0]);
}

void shell(void) {
    running = true;

    while (running) {
        u8 line[BUF_SIZE];
        u32 argc = 0;
        u8* argv[MAX_ARGS];

        mem_zero(line, sizeof(line));
        mem_zero(argv, sizeof(argv));

        print_prompt();
        get_line(line);
        parse_line(line, &argc, argv);
        exec_cmd(argc, argv);
    }
}

