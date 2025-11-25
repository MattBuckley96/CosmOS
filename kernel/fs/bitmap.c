#include "fs.h"
#include "printf.h"
#include "string.h"

void bitmap_reset(struct FsBitmap* bmp)
{
    u32 bytes = (BITMAP_SIZE + 7) / 8;

    for (u32 i = 0; i < bytes; i++)
    {
        bitmap_clear(bmp, i);
    }
}

void bitmap_set(struct FsBitmap* bmp, u32 index)
{
    bmp->data[index / 8] |= (1 << (index % 8)); 
}

void bitmap_clear(struct FsBitmap* bmp, u32 index)
{ 
    bmp->data[index / 8] &= ~(1 << (index % 8)); 
} 

int bitmap_get(struct FsBitmap* bmp, u32 index)
{
    return (bmp->data[index / 8] >> (index % 8)) & 1; 
}

u32 bitmap_find_free(struct FsBitmap* bmp)
{
    u32 bytes = (BITMAP_SIZE + 7) / 8;

    for (u32 i = 0; i < bytes; i++)
    {
        if (!bitmap_get(bmp, i))
            return (i + 1);
    }

    return 0;
}

void bitmap_print(struct FsBitmap* bmp)
{
    u32 bytes = (BITMAP_SIZE + 7) / 8;

    for (u32 i = 0; i < bytes; i++)
    {
        if (bitmap_get(bmp, i))
        {
            printf("1");
        }
        else
        {
            printf("0");
        }
    }
    printf("\n");
}
