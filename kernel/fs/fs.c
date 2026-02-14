#include "fs.h"
#include "drivers/ata.h"
#include "drivers/vga.h"
#include "printf.h"
#include "string.h"

///////////////////////////////////////////////

struct Superblock sb;
struct Descriptor desc;
struct FsBitmap   i_bmp;
struct FsBitmap   b_bmp;

///////////////////////////////////////////////

int fs_init(void)
{
    u8 buf[512];

    int err = ata_read(2, buf, 1);
    if (err)
        return FS_ERR_INIT;

    sb = *(struct Superblock*)buf;

    if (sb.magic != FS_MAGIC)
    {
        return FS_ERR_INIT;
    }

    err = ata_read(3, buf, 1);
    if (err)
        return FS_ERR_INIT;

    desc = *(struct Descriptor*)buf;

    err = ata_read(desc.inode_bitmap_addr, buf, 1);
    if (err)
        return err;

    i_bmp = *(struct FsBitmap*)buf;

    err = ata_read(desc.block_bitmap_addr, buf, 1);
    if (err)
        return err;

    b_bmp = *(struct FsBitmap*)buf;

    return FS_ERR_NONE;
}

int fs_sync(void)
{
    if (sb.state != FS_STATE_DIRTY)
        return FS_ERR_NONE;

    u8 buf[512];

    // block bitmap
    memcpy(buf, &b_bmp, sizeof(i_bmp));

    int err = ata_write(desc.block_bitmap_addr, &b_bmp, 1);
    if (err)
        return err;

    // inode bitmap
    memcpy(buf, &i_bmp, sizeof(i_bmp));

    err = ata_write(desc.inode_bitmap_addr, &i_bmp, 1);
    if (err)
        return err;

    // descriptor
    memcpy(buf, &desc, sizeof(desc));

    err = ata_write(3, buf, 1);
    if (err)
        return err;

    // superblock
    sb.state = FS_STATE_CLEAN;
    memcpy(buf, &sb, sizeof(sb));

    err = ata_write(2, buf, 1);
    if (err)
        return err;

    err = fs_init();
    if (err)
        return err;

    return FS_ERR_NONE;
}

int fs_create(void)
{
    sb = (struct Superblock) {
        .magic = FS_MAGIC,
        .inodes = MAX_INODES,
        .free_inodes = MAX_INODES,
        .blocks = MAX_BLOCKS,
        .free_blocks = MAX_BLOCKS,
        .block_size = 512,
        .state = FS_STATE_DIRTY,
    };

    desc = (struct Descriptor) {
        .inode_bitmap_addr = 4,
        .block_bitmap_addr = 5,
        .inodes_addr = 6,
        .blocks_addr = (6 + 512),
    };

    bitmap_reset(&i_bmp);
    bitmap_reset(&b_bmp);


    // create root
    {
        u32 inode = 0;

        int err = inode_alloc(&inode);
        if (err)
            return err;

        err = inode_set(inode, &(struct Inode){
            .size = 0,
            .type = FS_DIR,
            .blocks = {},
        });
        if (err)
            return err;

        err = inode_alloc_blocks(inode, 1);
        if (err)
            return err;

        u8 buf[sb.block_size];
        struct Dentry self = {
            .inode = inode,
            .type = FS_DIR,
            .name_len = 1,
            .name = "."
        };
        struct Dentry parent = {
            .inode = inode,
            .type = FS_DIR,
            .name_len = 2,
            .name = ".."
        };
        memcpy(buf, &self, sizeof(struct Dentry));
        memcpy(buf + sizeof(struct Dentry), &parent, sizeof(struct Dentry));

        struct File root = {
            .inode = 1,
        };

        // for testing
        err = file_write(&root, buf, sizeof(buf));
        if (err)
            return err;

        err = file_create(&root, "test", FS_FILE);
        if (err)
            return err;
    }

    int err = fs_sync();
    if (err)
        return err;

    return FS_ERR_NONE;
}
