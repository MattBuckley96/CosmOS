#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

u16* vga;
u8 cur_x;
u8 cur_y;
u8 cur_color;

inline u8 vga_color(u8 fg, u8 bg)
{
    return fg | bg << 4;
}

inline u16 vga_entry(char c, u8 color)
{
    return (u16)c | (u16)color << 8;
}

void vga_init(void)
{
    vga = (u16*)VGA_MEMORY;
    cur_x = 0;
    cur_y = 0;
    cur_color = vga_color(7, 0);

    for (u32 i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    {
        vga[i] = vga_entry(' ', cur_color);
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
            vga[cur_x++] = vga_entry(c, cur_color);
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
        // TODO: text scrolling
    }
}

void vga_write(const char* buf, u32 size)
{
    for (u32 i = 0; i < size; i++)
    {
        vga_putchar(buf[i]);
    }
}

void vga_print(const char* s)
{
    u32 len = 0;
    while (s[len])
    {
        len++;
    }

    vga_write(s, len);
}

void kmain(void)
{
    vga_init();
    vga_print("Hello, World!\n");
}