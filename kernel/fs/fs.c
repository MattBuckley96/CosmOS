#include "fs.h"
#include "drivers/ata.h"
#include "drivers/vga.h"
#include "printf.h"
#include "string.h"

///////////////////////////////////////////////

static struct Superblock sb;
static struct Descriptor desc;

///////////////////////////////////////////////

static inline struct Dentry* get_dentry_table(struct Inode* inode)
{
    u32 blocks = block_count(inode);
    u8 buf[blocks * 512];
    u8* offset = (u8*)buf;

    // read all blocks into buffer
    for (int i = 0; i < 10; i++)
    {
        if (inode->blocks[i] == 0)
            continue;
        
        u32 block = inode->blocks[i] - 1;

        int err = ata_read(desc.blocks_addr + block, offset, 1);
        if (err)
            return NULL;

        offset += 512;
    }

    struct Dentry* dentry = (struct Dentry*)buf;
    return dentry;
}

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

    return FS_ERR_NONE;
}

void fs_list(void)
{
    struct Inode root;

    int err = get_inode(1, &root);
    if (err)
        return;

    struct Dentry* dentry = get_dentry_table(&root);

    int count = (root.size / sizeof(struct Dentry));

    for (int i = 0; i < count; i++)
    {
        u8 col = VGA_LIGHT_GRAY;

        if (dentry[i].type == FS_DIR)
            col = VGA_LIGHT_CYAN;

        for (int j = 0; j < dentry[i].name_len; j++)
        {
            vga_putc(dentry[i].name[j], col);
        }
        printf("\n");
    }
}

///////////////////////////////////////////////

int get_inode(u32 inode, struct Inode* out)
{
    u8 buf[512];

    u32 inodes_per_block = (sb.block_size / sizeof(struct Inode));
    u32 block = (inode - 1) / inodes_per_block;

    int err = ata_read(desc.inodes_addr + block, buf, 1);
    if (err)
        return -1;

    u32 index = (inode - 1) % inodes_per_block;

    struct Inode* inodes = (struct Inode*)buf;

    *out = inodes[index];
    return 0;
}

u32 block_count(struct Inode* inode)
{
    u32 count = 0;

    // NOTE: only have 10 blocks for now, hardcoded
    for (int i = 0; i < 10; i++)
    {
        if (inode->blocks[i] > 0)
            count++;
    }

    return count;
}

///////////////////////////////////////////////

int file_open(struct File* file, const char* path)
{
    struct Inode root;

    int err = get_inode(1, &root);
    if (err)
        return FILE_ERR_OPEN;

    struct Dentry* dentry = get_dentry_table(&root);

    int count = (root.size / sizeof(struct Dentry));

    for (int i = 0; i < count; i++)
    {
        if (strcmp(path, dentry[i].name) == 0)
        {
            file->inode = dentry[i].inode;
            return FILE_ERR_NONE;
        }
    }

    return FILE_ERR_OPEN;
}

u32 file_get_size(struct File* file)
{
    struct Inode inode;
    get_inode(file->inode, &inode);

    return inode.size;
}

int file_read(struct File* file, void* out)
{
    struct Inode inode;

    int err = get_inode(file->inode, &inode);
    if (err)
        return FILE_ERR_READ;

    u32 blocks = block_count(&inode);
    u8 buf[512];
    u8* out_buf = (u8*)out;

    // NOTE: still hardcoded
    for (int i = 0; i < 10; i++)
    {
        if (inode.blocks[i] == 0)
            continue;

        u32 block = inode.blocks[i] - 1;
        err = ata_read(desc.blocks_addr + block, buf, 1);
        if (err)
            return FILE_ERR_READ;

        memcpy(out_buf, buf, 512);
        out_buf += 512;
    }

    return FILE_ERR_NONE;
}
