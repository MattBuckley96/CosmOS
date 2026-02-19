#include "fs.h"
#include "printf.h"
#include "string.h"
#include "drivers/ata.h"

///////////////////////////////////////////////////////////////////////////////

enum
{
    FILE_ERR_NONE   =  0,
    FILE_ERR_WRITE  = -1,
    FILE_ERR_APPEND = -2,
    FILE_ERR_CREATE = -3,
    FILE_ERR_OPEN   = -4,
};

///////////////////////////////////////////////////////////////////////////////

// TODO: switch to writing based off of remaining rather than the count
static int overwrite(struct File* file, const void* in, u32 size)
{
    struct Inode inode;

    int err = inode_get(file->inode, &inode);
    if (err)
        return err;

    u32 blocks = inode_block_count(&inode);

    float f_count = (size / (float)sb.block_size);
    u32 count = (u32)f_count;

    if ((f_count - count) > 0)
        count++;
 
    if (blocks < count)
    {
        err = inode_alloc_blocks(file->inode, (count - blocks));
        if (err)
            return err;

        int err = inode_get(file->inode, &inode);
        if (err)
            return err;
    }

    inode.size = size;
    u8* in_buf = (u8*)in;

    u32 remaining = size;

    // HACK: hardcoded 10 block size
    for (int i = 0; i < 10; i++)
    {
        u8 block_buf[sb.block_size] = {};

        if (count == 0)
            break;

        if (inode.blocks[i] == 0)
            continue;

        if (remaining > sb.block_size)
        {
            memcpy(block_buf, in_buf, sb.block_size);
            remaining -= sb.block_size;
        }
        else
        {
            memcpy(block_buf, in_buf, remaining);
        }

        u32 block = inode.blocks[i] - 1;

        err = ata_write(desc.blocks_addr + block, block_buf, 1);
        if (err)
            return err;

        in_buf += sb.block_size;
        count--;
    }

    err = inode_set(file->inode, &inode);
    if (err)
        return err;

    return FILE_ERR_NONE;
}

int append(struct File* file, const void* in, u32 count)
{
    // TODO: append 
    return FILE_ERR_APPEND;
}

///////////////////////////////////////////////////////////////////////////////

int file_read(struct File* file, void* out)
{
    struct Inode inode;

    int err = inode_get(file->inode, &inode);
    if (err)
        return err;

    u8* out_buf = (u8*)out;
    u32 remaining = inode.size;

    // HACK: still hardcoded
    for (int i = 0; i < 10; i++)
    {
        if (inode.blocks[i] == 0)
            continue;

        u8 block_buf[sb.block_size] = {};
        u32 block = inode.blocks[i] - 1;

        err = ata_read(desc.blocks_addr + block, block_buf, 1);
        if (err)
            return err;

        if (remaining > sb.block_size)
        {
            memcpy(out_buf, block_buf, sb.block_size);
            remaining -= sb.block_size;
        }
        else
        {
            memcpy(out_buf, block_buf, remaining);
        }

        out_buf += sb.block_size;
    }

    return FILE_ERR_NONE;
}

// writes in full blocks, but stores size
int file_write(struct File* file, const void* in, u32 size)
{
    u8 flags = file->flags;

    if (flags & IO_APPEND)
        return FILE_ERR_WRITE;

    return overwrite(file, in, size);
}

int file_create(struct File* dir, const char* name, u8 type)
{
    struct Inode dir_inode;

    int err = inode_get(dir->inode, &dir_inode);
    if (err)
        return err;

    u32 count = (dir_inode.size / sizeof(struct Dentry));
    struct Dentry* table = inode_dentry_table(&dir_inode);
    if (!table)
        return FILE_ERR_CREATE;

    for (int i = 0; i < count; i++)
    {
        if (strcmp(name, table[i].name) == 0)
            return FILE_ERR_CREATE;
    }

    u32 inode = 0;

    err = inode_alloc(&inode);
    if (err)
        return err;

    err = inode_set(inode, &(struct Inode){
        .type = type,
        .size = 0,
        .blocks = {}
    });
    if (err)
        return err;

    err = inode_alloc_blocks(inode, 1);
    if (err)
        return err;

    struct Dentry dentry = {
        .inode = inode,
        .type = type,
        .name_len = strlen(name)
    };
    strcpy(dentry.name, name);

    err = dentry_create(dir, &dentry);
    if (err)
        return err;

    if (type == FS_DIR)
    {
        struct File file = {
            .inode = inode
        };

        err = dentry_create(&file, &(struct Dentry){
            .inode = inode,
            .type = FS_DIR,
            .name_len = 1,
            .name = "."
        });
        if (err)
            return err;

        err = dentry_create(&file, &(struct Dentry){
            .inode = dir->inode,
            .type = FS_DIR,
            .name_len = 2,
            .name = ".."
        });
        if (err)
            return err;
    }

    return FILE_ERR_NONE;
}

int file_open(struct File* dir, const char* name, struct File* file, u8 flags)
{
    struct Inode dir_inode;

    int err = inode_get(dir->inode, &dir_inode);
    if (err)
        return err;

    struct Dentry* table = inode_dentry_table(&dir_inode);
    if (!table)
        return FILE_ERR_OPEN;

    u32 count = (dir_inode.size / sizeof(struct Dentry)); 

    for (int i = 0; i < count; i++)
    {
        if (strcmp(name, table[i].name) == 0)
        {
            file->inode = table[i].inode;
            file->flags = flags;

            return FILE_ERR_NONE;
        }
    }

    return FILE_ERR_OPEN;
}
