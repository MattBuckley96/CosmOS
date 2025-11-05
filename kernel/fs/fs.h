#ifndef FS_H
#define FS_H

#include "types.h"

///////////////////////////////////////////////

#define FS_MAGIC 0x42

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
    u16 magic;
    u32 inodes;
    u32 free_inodes;
    u32 blocks;
    u32 free_blocks;
    u32 block_size;
} PACKED;

struct Descriptor
{
    u32 inodes_addr;
    u32 blocks_addr;
} PACKED;

struct Inode
{
    u16 uid;
    u32 size;
    u32 blocks[10];
    u8  type;
    u8  padding[17];
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
