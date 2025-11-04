#ifndef FS_H
#define FS_H

#include "types.h"

///////////////////////////////////////////////

#define FS_MAGIC 67

///////////////////////////////////////////////

enum
{
    FS_ERR_NONE =  0,
    FS_ERR_INIT = -1,
};

enum
{
    FS_FILE = 1,
    FS_DIR  = 2
};

enum
{
    FILE_ERR_NONE =  0,
    FILE_ERR_OPEN = -1,
};

///////////////////////////////////////////////

struct Superblock
{
    u32 inodes;
    u32 free_inodes;
    u32 blocks;
    u32 free_blocks;
    u32 block_size;
    u16 magic;
} PACKED;

struct Descriptor
{
    u32 block_bitmap_addr;
    u32 inode_bitmap_addr;
    u32 inodes_addr;
    u16 dentries;
    u32 dentries_addr;
} PACKED;

struct Inode
{
    u16 uid;
    u32 size;
    u32* blocks[10];
    u8 padding[18];
} PACKED;

struct Dentry
{
    char name[256];
    u8  name_len;
    u16 inode;
    u8  type;
} PACKED;

struct File
{
    u16 inode;
};

///////////////////////////////////////////////

int fs_init(void);
void fs_list(void);

int file_open(struct File* file, const char* path);
u32 file_get_size(struct File* file);

///////////////////////////////////////////////

#endif
