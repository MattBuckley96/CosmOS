#include "fs.h"
#include "drivers/ata.h"
#include "drivers/vga.h"
#include "printf.h"
#include "string.h"

///////////////////////////////////////////////

static struct Superblock sb;
static struct Descriptor desc;

///////////////////////////////////////////////

static inline struct Dentry* get_dentry_table(void)
{
    u32 count = ((sizeof(struct Dentry) * desc.dentries) / 512) + 1;
    u8 buf[count * 512];

    int err = ata_read(desc.dentries_addr, buf, count);
    if (err)
        return NULL;

    struct Dentry* dentry = (struct Dentry*)buf;
    return dentry;
}

///////////////////////////////////////////////

int fs_init(void)
{
    u8 buf[512];

    ata_read(2, buf, 1);
    sb = *(struct Superblock*)buf;

    if (sb.magic != FS_MAGIC)
        return FS_ERR_INIT;

    ata_read(3, buf, 1);
    desc = *(struct Descriptor*)buf;

    return FS_ERR_NONE;
}

void fs_list(void)
{
    struct Dentry* dentry = get_dentry_table();

    for (int i = 0; i < desc.dentries; i++)
    {
        u8 col = VGA_LIGHT_GRAY;

        if (dentry->type == FS_DIR)
            col = VGA_LIGHT_YELLOW;

        for (int j = 0; j < dentry->name_len; j++)
        {
            vga_putc(dentry->name[j], col);
        }
        printf("\n");

        dentry++;
    }
}

///////////////////////////////////////////////

int file_open(struct File* file, const char* path)
{
    struct Dentry* dentry = get_dentry_table();

    for (int i = 0; i < desc.dentries; i++)
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
    u8 buf[512];

    int err = ata_read(desc.inodes_addr, buf, 1);
    if (err)
        return 0;

    struct Inode* inodes = (struct Inode*)buf;

    return inodes[file->inode - 1].size;
}
