#include "types.h"
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

static u16 entry(char c)
{
    u16 attr = (0x00) | (7 & 0x0F);
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
        vga.buf[i] = entry(' ');
    }

    vga.cx = 0;
    vga.cy = 0;
}

void vga_putc(char c)
{
    u8 index = vga.cy * VGA_WIDTH + vga.cx;

    switch (c)
    {
    case '\n':
        if (vga.cx > 0) vga.cx--;
        vga.buf[index] = entry(' ');
        break;
    default:
        vga.buf[index] = entry(c);
        vga.cx++;
        break;
    }

    if (vga.cx >= VGA_WIDTH)
    {
        vga.cx = 0;
        vga.cy++;
    }
}

void vga_puts(const char* s)
{
    while (*s)
    {
        vga_putc(*s++);
    }
}
