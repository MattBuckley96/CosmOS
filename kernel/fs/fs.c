#include "fs.h"
#include "drivers/ata.h"
#include "drivers/vga.h"
#include "printf.h"
#include "string.h"

///////////////////////////////////////////////

static struct Superblock sb;
static struct Descriptor desc;

///////////////////////////////////////////////

static int get_inode(u32 inode, struct Inode* out)
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

// HACK: 
// need to calculate block count, then traverse, 
// since they might be fragmented
static struct Dentry* get_dentry_table(struct Inode* inode)
{
    if (inode->type != FS_DIR)
        return NULL;

    int addr = desc.blocks_addr + (inode->blocks[0] - 1);
    int sectors = (inode->size / 512) + 1;

    u8 buf[sectors * 512];

    int err = ata_read(addr, buf, sectors);
    if (err)
        return NULL;

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
