#include "fs.h"
#include "string.h"
#include "drivers/ata.h"

///////////////////////////////////////////////

enum
{
    FILE_ERR_NONE   =  0,
    FILE_ERR_OPEN   = -1,
    FILE_ERR_READ   = -2,
    FILE_ERR_WRITE  = -3,
    FILE_ERR_CREATE = -4,
};

///////////////////////////////////////////////

int file_open(struct File* file, struct Dir* dir, const char* path)
{
    struct Inode inode;

    int err = inode_get(dir->inode, &inode);
    if (err)
        return err;

    struct Dentry* dentry = inode_dentry_table(&inode);
    int count = (inode.size / sizeof(struct Dentry));

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

int file_read(struct File* file, void* out)
{
    struct Inode inode;

    int err = inode_get(file->inode, &inode);
    if (err)
        return err;

    u32 blocks = inode_block_count(&inode);
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
            return err;

        memcpy(out_buf, buf, 512);
        out_buf += 512;
    }

    return FILE_ERR_NONE;
}

// write in full blocks (for now)
int file_write(struct File* file, void* in, u32 count)
{
    struct Inode inode;

    int err = inode_get(file->inode, &inode);
    if (err)
        return err;

    u32 blocks = inode_block_count(&inode);
    u8 buf[512];
    u8* in_buf = (u8*)in;

    // NOTE: eventually alloc more blocks
    if (count > blocks)
        return FILE_ERR_WRITE;

    // NOTE: hardcoded 10 block size
    for (int i = 0 ; i < 10; i++)
    {
        if (count == 0)
            break;

        if (inode.blocks[i] == 0)
            continue;

        memcpy(buf, in_buf, 512);

        u32 block = inode.blocks[i] - 1;

        err = ata_write(desc.blocks_addr + block, buf, 1);
        if (err)
            return err;

        in_buf += 512;
        count--;
    }

    return FILE_ERR_NONE;
}
