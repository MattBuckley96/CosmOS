#include "types.h"
#include "io.h"

#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

u16* vga;
u16 cur_x;
u16 cur_y;
u8 cur_color;

inline u8 vga_color(u8 fg, u8 bg)
{
    return fg | bg << 4;
}

inline u16 vga_entry(char c, u8 color)
{
    return (u16)c | (u16)color << 8;
}

void cursor_enable(u8 start, u8 end)
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | start);

    outb(0x3D5, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | end);
}

void cursor_disable(void)
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void cursor_update(u8 x, u8 y)
{
    u16 pos = y * VGA_WIDTH + x;
    
    outb(0x3D4, 0x0F);
    outb(0x3D5, (u8)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (u8)((pos >> 8) & 0xFF));
}

void vga_init(void)
{
    vga = (u16*)VGA_MEMORY;
    cur_x = 0;
    cur_y = 0;
    cur_color = vga_color(7, 0);

    cursor_enable(0, 0);

    for (u32 i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    {
        vga[i] = vga_entry(' ', cur_color);
    }
}

void vga_scroll(void)
{
    for (u16 y = 0; y < VGA_HEIGHT; y++)
    {
        for (u16 x = 0; x < VGA_WIDTH; x++)
        {
            u16 idx = y * VGA_WIDTH + x;
            vga[(y - 1) * VGA_WIDTH + x] = vga[idx];
        }
    }

    for (u16 x = 0; x < VGA_WIDTH; x++)
    {
        u16 idx = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        vga[idx] = vga_entry(' ', cur_color);
    }
}

void vga_putchar(char c)
{
    switch (c)
    {
        case '\n':
        {
            cur_x = 0;
            cur_y++;
            break;
        }

        default:
        {
            u16 idx = cur_y * VGA_WIDTH + cur_x;
            vga[idx] = vga_entry(c, cur_color);
            cur_x++;
            break;
        }
    }

    if (cur_x >= VGA_WIDTH)
    {
        cur_x = 0;
        cur_y++;
    }

    if (cur_y >= VGA_HEIGHT)
    {
        vga_scroll();
        cur_y = VGA_HEIGHT - 1;
    }

    cursor_update(cur_x, cur_y);
}

void vga_print(const char* s)
{
    for (u32 i = 0; s[i]; i++)
    {
        vga_putchar(s[i]);
    }
}

void kmain(void)
{
    vga_init();
    vga_print("Hello, World!\n");
}