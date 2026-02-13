#include "fs.h"

///////////////////////////////////////////////

enum
{
    BLOCK_ERR_NONE  =  0,
    BLOCK_ERR_ALLOC = -1,
};

///////////////////////////////////////////////

int block_alloc(u32* block)
{
    if (sb.free_blocks == 0)
        return BLOCK_ERR_ALLOC;

    u32 free = bitmap_find_free(&b_bmp);
    if (free == 0)
        return BLOCK_ERR_ALLOC;

    bitmap_set(&b_bmp, (free - 1));

    sb.free_blocks--;
    sb.state = FS_STATE_DIRTY;

    int err = fs_sync();
    if (err)
        return err;

    *block = free;

    return BLOCK_ERR_NONE;
}
