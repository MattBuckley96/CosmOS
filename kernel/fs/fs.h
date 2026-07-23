#pragma once

#include "types.h"

#define FS_FILE 1
#define FS_DIR 2

#pragma pack(1)
typedef struct superblock {
    u8 magic[8];
    u32 inodes_count;
    u32 free_inodes_count;
    u32 blocks_count;
    u32 free_blocks_count;
    u32 block_bitmap;
    u32 inode_bitmap;
    u32 inode_table;
    u32 first_data_block;
} superblock_t;

typedef struct inode {
    u8 type;
    u32 size;
    u8 padding[7];
    u32 direct[12];
    u32 indirect;
} inode_t;

typedef struct dentry {
    u16 entry_len;
    u32 id;
    u8 type;
    u8 name_len;
} dentry_t;
#pragma pack()

void fs_create(void);
void fs_init(void);
void fs_print(void);
void fs_update(void);

void bitmap_set_bit(u8* bitmap, u32 bit);
void bitmap_clear_bit(u8* bitmap, u32 bit);
bool bitmap_get_bit(u8* bitmap, u32 bit);

u32 fs_alloc_inode(void);
void fs_free_inode(u32 id);
void fs_get_inode(u32 id, inode_t* out);
void fs_update_inode(u32 id, inode_t* in);

u32 fs_alloc_block(void);
void fs_free_block(u32 block);

u32 inode_get_block_count(u32 id);
void inode_alloc_blocks(u32 id, u32 count);
u32 inode_write(u32 id, void* in, u32 size);
u32 inode_read(u32 id, void* out, u32 size);

void fs_add_dir_entry(u32 id, dentry_t* dentry, const u8* name);
void fs_list_dir(u32 id);
u32 fs_create_dir(u32 parent_id, const u8* name);
u32 fs_dir_find(u32, const u8* name);
