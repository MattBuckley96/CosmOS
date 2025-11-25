#include "fs.h"
#include "printf.h"
#include "drivers/vga.h"

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

    int err = inode_get(dir->inode, &inode);
    if (err)
        return;

    struct Dentry* dentry = inode_dentry_table(&inode);
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

    int err = inode_get(dir->inode, &inode);
    if (err)
        return NULL;

   struct Dentry* dentry = inode_dentry_table(&inode); 
   if (!dentry)
       return NULL;

   err = inode_get(dentry[1].inode, &inode);
   if (err)
       return NULL;

   dentry = inode_dentry_table(&inode);
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
