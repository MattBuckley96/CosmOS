#include "shell.h"
#include "fs/fs.h"
#include "printf.h"
#include "string.h"
#include "drivers/ata.h"

void shell_mkfs(int argc, char** argv)
{
    struct Superblock sb = {
        .magic = FS_MAGIC,
        .inodes = 4096,
        .free_inodes = 4094,
        .blocks = 4096,
        .free_blocks = 4094,
        .block_size = 512,
    };

    struct Descriptor desc = {
        .inodes_addr = 7,
        .blocks_addr = (7 + 512),
    };

    struct Inode inodes[] = {
        (struct Inode) {
            .size = sizeof(struct Dentry) * 2,
            .type = FS_DIR,
            .blocks = { 1, 2, 0, 0, 0, 0, 0, 0, 0, 0 }
        },
        (struct Inode) {
            .size = 0,
            .type = FS_DIR,
            .blocks = {}
        },
        (struct Inode) {
            .size = 67,
            .type = FS_FILE,
            .blocks = { 3, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
        }
    };

    struct Dentry root_entries[] = {
        (struct Dentry) {
            .name = "dir",
            .name_len = 3,
            .inode = 2,
            .type = FS_DIR
        },
        (struct Dentry) {
            .name = "test.txt",
            .name_len = 8,
            .inode = 3,
            .type = FS_FILE 
        },
    };

    // memcpy so that junk is not written to disk
    u8 buf[512];
    memcpy(buf, &sb, sizeof(sb));

    int err = ata_write(2, buf, 1);
    if (err)
        return;

    memcpy(buf, &desc, sizeof(desc));

    err = ata_write(3, buf, 1);
    if (err)
        return;

    memcpy(buf, &inodes, sizeof(inodes));

    err = ata_write(desc.inodes_addr, buf, 1);
    if (err)
        return;

    // HACK: this shit is so scuffed but it works for now
    u8 entries_buf[1024];
    memcpy(entries_buf, &root_entries, 1024); 

    err = ata_write(desc.blocks_addr, entries_buf, 2);
    if (err)
        return;

    u8 test_buf[512];
    memcpy(test_buf, "hello from test", 16);

    u32 block = inodes[2].blocks[0] - 1;

    err = ata_write(desc.blocks_addr + block, test_buf, 1);
    if (err)
        return;

    fs_init();
}
