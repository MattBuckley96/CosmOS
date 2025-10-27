#include "vga.h"

///////////////////////////////////////////////

struct Vga
{
    u16* buf;
    u8 cx;
    u8 cy;
};

///////////////////////////////////////////////

static struct Vga vga;

///////////////////////////////////////////////

static u16 entry(char c, u8 col)
{
    u16 attr = (0x00) | (col & 0x0F);
    return ((u8)c | (attr << 8));
}

///////////////////////////////////////////////

void vga_init(void)
{
    vga.buf = (u16*)VGA_MEMORY; 
    vga.cx = 0;
    vga.cy = 0;
}

void vga_clear(void)
{
    for (u16 i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    {
        vga.buf[i] = entry(' ', VGA_BLACK);
    }

    vga.cx = 0;
    vga.cy = 0;
}

void vga_putc(char c, u8 col)
{
    u16 index = vga.cy * VGA_WIDTH + vga.cx;

    switch (c)
    {
    case '\n':
        vga.cx = 0;
        vga.cy++;
        break;
    case '\b':
        if (vga.cx > 0) vga.cx--;
        vga.buf[index] = entry(' ', VGA_BLACK);
        break;
    default:
        vga.buf[index] = entry(c, col);
        vga.cx++;
        break;
    }

    if (vga.cx >= VGA_WIDTH)
    {
        vga.cx = 0;
        vga.cy++;
    }
}

void vga_puts(const char* s, u8 col)
{
    while (*s)
    {
        vga_putc(*s++, col);
    }
}
