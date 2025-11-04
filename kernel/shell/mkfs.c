#include "shell.h"
#include "fs/fs.h"
#include "printf.h"
#include "drivers/ata.h"

void shell_mkfs(int argc, char** argv)
{
    struct Superblock sb = {
        .inodes = 8,
        .free_inodes = 6,
        .blocks = 10,
        .free_blocks = 10,
        .block_size = 512,
        .magic = 67,
    };

    struct Descriptor desc = {
        .inode_bitmap_addr = 0,
        .block_bitmap_addr = 0,
        .dentries = 2,
        .dentries_addr = 4,

        // hard coded for now
        .inodes_addr = 10
    };

    struct Dentry root[] = {
        (struct Dentry) {
            .name = { "/" },
            .name_len = 1,
            .inode = 1,
            .type = FS_DIR,
        },
        (struct Dentry) {
            .name = { "test" },
            .name_len = 4,
            .inode = 2,
            .type = FS_FILE
        },
    };

    int err = ata_write(2, (u8*)&sb, 1);
    if (err)
        return;

    err = ata_write(3, (u8*)&desc, 1);
    if (err)
        return;

    u32 count = (sizeof(root) / 512) + 1;

    err = ata_write(desc.dentries_addr, (u8*)root, count);
    if (err)
        return;

    struct Inode inodes[] = {
        (struct Inode) {
            .uid = 1,
            .blocks = NULL,
            .size = 0,
        },
        (struct Inode) {
            .uid = 2,
            .blocks = NULL,
            .size = 67,
        }
    };

    err = ata_write(desc.inodes_addr, (u8*)&inodes, 1);
    if (err)
        return;

    fs_init();
}
