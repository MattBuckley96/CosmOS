#include "fs.h"
#include "drivers/ata.h"
#include "drivers/vga.h"
#include "printf.h"
#include "string.h"

///////////////////////////////////////////////

static struct Superblock sb;
static struct Descriptor desc;

///////////////////////////////////////////////

static inline struct Dentry* get_dentry_table(struct Inode* inode)
{
    u32 blocks = block_count(inode);
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

int fs_sync(void)
{
    if (sb.state != FS_STATE_DIRTY)
        return FS_ERR_NONE;

    sb.state = FS_STATE_CLEAN;

    u8 buf[512];

    memcpy(buf, &sb, sizeof(sb));

    int err = ata_write(2, buf, 1);
    if (err)
        return err;

    memcpy(buf, &desc, sizeof(desc));

    err = ata_write(3, buf, 1);
    if (err)
        return err;

    err = fs_init();
    if (err)
        return err;

    return FS_ERR_NONE;
}

///////////////////////////////////////////////

int dir_open(struct Dir* dir, const char* path)
{
    struct File* file = (struct File*)dir;

    int err = file_open(file, dir, path);
    if (err)
        return err;

    return DIR_ERR_NONE;
}

void dir_list(struct Dir* dir)
{
    struct Inode inode;

    int err = get_inode(dir->inode, &inode);
    if (err)
        return;

    struct Dentry* dentry = get_dentry_table(&inode);
    if (!dentry)
        return;

    int count = (inode.size / sizeof(struct Dentry));

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

char* dir_name(struct Dir* dir)
{
    if (dir->inode == 1)
        return NULL;

    struct Inode inode;

    int err = get_inode(dir->inode, &inode);
    if (err)
        return NULL;

   struct Dentry* dentry = get_dentry_table(&inode); 
   if (!dentry)
       return NULL;

   err = get_inode(dentry[1].inode, &inode);
   if (err)
       return NULL;

   dentry = get_dentry_table(&inode);
   if (!dentry)
       return NULL;

   int count = (inode.size / sizeof(struct Dentry));

   for (int i = 0; i < count; i++)
   {
       if (dentry[i].inode == dir->inode)
           return dentry[i].name;
   }

   return NULL;
}

///////////////////////////////////////////////

int get_inode(u32 inode, struct Inode* out)
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

u32 block_count(struct Inode* inode)
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

///////////////////////////////////////////////

int file_open(struct File* file, struct Dir* dir, const char* path)
{
    struct Inode inode;

    int err = get_inode(dir->inode, &inode);
    if (err)
        return err;

    struct Dentry* dentry = get_dentry_table(&inode);
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

u32 file_get_size(struct File* file)
{
    struct Inode inode;
    get_inode(file->inode, &inode);

    return inode.size;
}

int file_read(struct File* file, void* out)
{
    struct Inode inode;

    int err = get_inode(file->inode, &inode);
    if (err)
        return err;

    u32 blocks = block_count(&inode);
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

    int err = get_inode(file->inode, &inode);
    if (err)
        return err;

    u32 blocks = block_count(&inode);
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

///////////////////////////////////////////////

int fs_create(void)
{
    sb = (struct Superblock) {
        .magic = FS_MAGIC,
        .inodes = 4096,
        .free_inodes = 4090,
        .blocks = 4096,
        .free_blocks = 4090,
        .block_size = 512,
        .state = FS_STATE_DIRTY,
    };

    desc = (struct Descriptor) {
        .inodes_addr = 7,
        .blocks_addr = (7 + 512),
    };

    struct Inode inodes[] = {
        (struct Inode) {
            .size = sizeof(struct Dentry) * 2,
            .type = FS_DIR,
            .blocks = { 1, 2, 0, 0, 0, 0, 0, 0, 0, 0 }
        },
        (struct Inode) {
            .size = sizeof(struct Dentry) * 3,
            .type = FS_DIR,
            .blocks = { 3, 4, 0, 0, 0, 0, 0, 0, 0, 0 }
        },
        (struct Inode) {
            .size = 16,
            .type = FS_FILE,
            .blocks = { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
        },
        (struct Inode) {
            .size = 16,
            .type = FS_FILE,
            .blocks = { 6, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
        }
    };

    struct Dentry root_entries[] = {
        (struct Dentry) {
            .name = "dir",
            .name_len = 3,
            .inode = 2,
            .type = FS_DIR
        },
        (struct Dentry) {
            .name = "test.txt",
            .name_len = 8,
            .inode = 3,
            .type = FS_FILE 
        },
    };

    struct Dentry dir_entries[] = {
        (struct Dentry) {
            .name = ".",
            .name_len = 1,
            .inode = 2,
            .type = FS_DIR
        },
        (struct Dentry) {
            .name = "..",
            .name_len = 2,
            .inode = 1,
            .type = FS_DIR
        },
        (struct Dentry) {
            .name = "deep.txt",
            .name_len = 8,
            .inode = 4,
            .type = FS_FILE
        }
    };

    // finally less scuffec
    int err = fs_sync();
    if (err)
        return err;

    // memcpy so that junk is not written to disk
    u8 buf[512];

    memcpy(buf, &inodes, sizeof(inodes));

    err = ata_write(desc.inodes_addr, buf, 1);
    if (err)
        return err;

    // HACK: this shit is so scuffed but it works for now
    u8 entries_buf[512 * 4];
    memcpy(entries_buf, &root_entries, 1024); 
    memcpy(entries_buf + 1024, &dir_entries, 1024);

    err = ata_write(desc.blocks_addr, entries_buf, 4);
    if (err)
        return err;

    // test.txt
    memcpy(buf, "hello from test", inodes[2].size);

    u32 block = inodes[2].blocks[0] - 1;

    err = ata_write(desc.blocks_addr + block, buf, 1);
    if (err)
        return err;

    // deep.txt
    memcpy(buf, "hello from deep", inodes[3].size);

    block = inodes[3].blocks[0] - 1;

    err = ata_write(desc.blocks_addr + block, buf, 1);
    if (err)
        return err;

    err = fs_init();
    if (err)
        return err;

    return FS_ERR_NONE;
}
