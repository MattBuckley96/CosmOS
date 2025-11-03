#include "fs.h"
#include "drivers/ata.h"
#include "drivers/vga.h"
#include "printf.h"

///////////////////////////////////////////////

static struct Superblock sb;
static struct Descriptor desc;

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
    u8 buf[512];

    u32 count = ((sizeof(struct Dentry) * desc.dirs) / 512) + 1;
    u8 dentry_buf[count * 512];

    ata_read(desc.dirs_addr, dentry_buf, count);

    struct Dentry* dentry = (struct Dentry*)dentry_buf;

    for (int i = 0; i < desc.dirs; i++)
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

u32 fs_get_size(u32 uid)
{
    u8 buf[512];

    int err = ata_read(desc.inodes_addr, buf, 1);
    if (err)
        return 0;

    struct Inode* inode = (struct Inode*)buf;

    for (int i = 0; i < sb.inodes; i++)
    {
        if (inode->uid != uid)
        {
            inode++;
            continue;
        }

        return inode->size;
    }

    return 0;
}
