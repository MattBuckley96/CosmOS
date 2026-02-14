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

    return INODE_ERR_NONE;
}

int inode_set(u32 inode, struct Inode* in)
{
    u8 buf[sb.block_size];

    u32 inodes_per_block = (sb.block_size / sizeof(struct Inode));
    u32 block = (inode - 1) / inodes_per_block;

    int err = ata_read(desc.inodes_addr + block, buf, 1);
    if (err)
        return -1;

    u32 index = (inode - 1) % inodes_per_block;

    struct Inode* inodes = (struct Inode*)buf;

    inodes[index] = *in;

    err = ata_write(desc.inodes_addr + block, buf, 1);
    if (err)
        return err;

    return INODE_ERR_NONE;
}

int inode_alloc(u32* inode)
{
    if (sb.free_inodes == 0)
        return INODE_ERR_ALLOC;

    u32 free = bitmap_find_free(&i_bmp);
    if (free == 0)
        return INODE_ERR_ALLOC;

    bitmap_set(&i_bmp, (free - 1));

    sb.free_inodes--;
    sb.state = FS_STATE_DIRTY;

    int err = fs_sync();
    if (err)
        return err;

    *inode = free;

    return INODE_ERR_NONE;
}

int inode_alloc_blocks(u32 inode, u32 count)
{
    if (count == 0)
        return INODE_ERR_NONE;

    struct Inode s_inode;

    int err = inode_get(inode, &s_inode);
    if (err)
        return err;

    while (count > 0)
    {
        u32 blocks = inode_block_count(&s_inode);

        if (blocks >= 10)
            return INODE_ERR_ALLOC;

        u32 block = 0;

        err = block_alloc(&block);
        if (err)
            return INODE_ERR_ALLOC;

        s_inode.blocks[blocks] = block;

        err = inode_set(inode, &s_inode);
        if (err)
            return INODE_ERR_ALLOC;

        count--;
    }

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
    u8 buf[blocks * sb.block_size];
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

        offset += sb.block_size;
    }

    struct Dentry* dentry = (struct Dentry*)buf;
    return dentry;
}
