#ifndef FS_H
#define FS_H

#include "types.h"

///////////////////////////////////////////////////////////////////////////////

#define FS_MAGIC 0x42

#define MAX_BLOCKS  (8 * 512)
#define MAX_INODES  (8 * 512)
#define BITMAP_SIZE (8 * 512)

///////////////////////////////////////////////////////////////////////////////

enum
{
    FS_ERR_NONE   =  0,
    FS_ERR_INIT   = -1,
    FS_ERR_CREATE = -2,
};

enum
{
    FS_STATE_CLEAN =  1,
    FS_STATE_DIRTY =  2,
};

enum
{
    FS_FILE = 1,
    FS_DIR  = 2
};

enum
{
    IO_APPEND = (1 << 0),
};

///////////////////////////////////////////////////////////////////////////////

struct Superblock
{
    u16 magic;
    u32 inodes;
    u32 free_inodes;
    u32 blocks;
    u32 free_blocks;
    u32 block_size;
    u8  state;
} PACKED;

struct Descriptor
{
    u32 inode_bitmap_addr;
    u32 block_bitmap_addr;
    u32 inodes_addr;
    u32 blocks_addr;
} PACKED;

struct Inode
{
    u32 size;
    u32 blocks[10];
    u8  type;
    u8  padding[19];
} PACKED;

struct Dentry
{
    u16 inode;
    u8  type;
    u8  name_len;
    char name[252];
} PACKED;

struct File
{
    u16 inode;
    u8  flags;
};

struct FsBitmap
{
    u8 data[(BITMAP_SIZE + 7) / 8]; 
};

///////////////////////////////////////////////////////////////////////////////

extern struct Superblock sb;
extern struct Descriptor desc;
extern struct FsBitmap   i_bmp;
extern struct FsBitmap   b_bmp;

///////////////////////////////////////////////////////////////////////////////

// fs.c
int fs_create(void);
int fs_init(void);
int fs_sync(void);

// bitmap.c
void bitmap_reset(struct FsBitmap* bmp);
void bitmap_set(struct FsBitmap* bmp, u32 index);
void bitmap_clear(struct FsBitmap* bmp, u32 index);
int bitmap_get(struct FsBitmap* bmp, u32 index);
u32 bitmap_find_free(struct FsBitmap* bmp); 
void bitmap_print(struct FsBitmap* bmp);

// inode.c
int inode_get(u32 inode, struct Inode* out);
int inode_set(u32 inode, struct Inode* in);
int inode_alloc(u32* inode);
int inode_alloc_blocks(u32 inode, u32 count);
u32 inode_block_count(struct Inode* inode);
struct Dentry* inode_dentry_table(struct Inode* inode);

// block.c
int block_alloc(u32* block);

// file.c
int file_read(struct File* file, void* out);
int file_write(struct File* file, const void* in, u32 count);
int file_create(struct File* dir, const char* name, u8 type);

// dir.c
void dir_list(struct File* dir);
int dentry_create(struct File* dir, struct Dentry* dentry);

///////////////////////////////////////////////////////////////////////////////

#endif
