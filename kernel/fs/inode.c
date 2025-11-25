#include "fs.h"
#include "drivers/ata.h"

///////////////////////////////////////////////

enum
{
    INODE_ERR_NONE  =  0,
    INODE_ERR_ALLOC = -1,
};


///////////////////////////////////////////////

int inode_get(u32 inode, struct Inode* out)
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

int inode_set(u32 inode, struct Inode* in)
{
    return INODE_ERR_NONE;
}

int inode_alloc(u32* inode)
{
    if (sb.free_inodes == 0)
        return INODE_ERR_ALLOC;

    u32 free = bitmap_find_free(&i_bmp);
    if (free == 0)
        return INODE_ERR_NONE;

    bitmap_set(&i_bmp, (free - 1));

    sb.free_inodes--;

    int err = fs_sync();
    if (err)
        return err;

    *inode = free;

    return INODE_ERR_NONE;
}

u32 inode_block_count(struct Inode* inode)
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

struct Dentry* inode_dentry_table(struct Inode* inode)
{
    u32 blocks = inode_block_count(inode);
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
