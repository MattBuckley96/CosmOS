#include "fs.h"
#include "printf.h"
#include "string.h"
#include "drivers/vga.h"

///////////////////////////////////////////////////////////////////////////////

enum
{
    DIR_ERR_NONE = 0,
};

enum
{
    DENTRY_ERR_NONE   =  0,
    DENTRY_ERR_CREATE = -1,
};

///////////////////////////////////////////////////////////////////////////////

void dir_list(struct File* dir)
{
    struct Inode inode;

    int err = inode_get(dir->inode, &inode);
    if (err)
        return;

    struct Dentry* table = inode_dentry_table(&inode);
    if (!table)
        return;

    u32 count = (inode.size / sizeof(struct Dentry));

    for (int i = 0; i < count; i++)
    {
        u8 col = VGA_LIGHT_GRAY;

        if (table[i].type == FS_DIR)
        {
            col = VGA_LIGHT_CYAN;
        }

        for (int j = 0; j < table[i].name_len; j++)
        {
            vga_putc(table[i].name[j], col);
        }
        printf("\n");
    }
}

int dentry_create(struct File* dir, struct Dentry* dentry)
{
    struct Inode inode;

    int err = inode_get(dir->inode, &inode);
    if (err)
        return err;

    u32 dentry_count = (inode.size / sizeof(struct Dentry));
    u32 index = dentry_count;

    u32 d_needed = dentry_count + 1;

    u32 blocks = inode_block_count(&inode);
    u32 size_needed = (d_needed * sizeof(struct Dentry));

    if ((blocks * sb.block_size) < size_needed)
    {
        err = inode_alloc_blocks(dir->inode, 1);
        if (err)
            return err;

        err = inode_get(dir->inode, &inode);
        if (err)
            return err;

        blocks = inode_block_count(&inode);
    }

    u8 buf[blocks * sb.block_size] = {};
 
    err = file_read(dir, buf);
    if (err)
        return err;
    
    struct Dentry* table = (struct Dentry*)buf;
    table[index] = *dentry;

    u32 size = (dentry_count + 1) * sizeof(struct Dentry);
    err = file_write(dir, buf, size);
    if (err)
        return err;

    return DENTRY_ERR_NONE;
}
