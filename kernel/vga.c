#include "vga.h"
#include "io.h"
#include <stdarg.h>

u16* vga;
u16 cur_x;
u16 cur_y;
u8 color;

void vga_init(void)
{
    vga = (u16*)VGA_MEMORY;
    cur_x = 0;
    cur_y = 0;
    color= vga_color(VGA_GRAY, VGA_BLACK);

    cursor_enable(0, 14);

    for (u32 i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = vga_entry(' ', color);
    }
}

void vga_set_color(u8 fg, u8 bg)
{
    color = vga_color(fg, bg);
}

void vga_scroll(void)
{
    for (u16 y = 0; y < VGA_HEIGHT; y++) {
        for (u16 x = 0; x < VGA_WIDTH; x++) {
            u16 idx = y * VGA_WIDTH + x;
            vga[(y - 1) * VGA_WIDTH + x] = vga[idx];
        }
    }

    for (u16 x = 0; x < VGA_WIDTH; x++) {
        u16 idx = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        vga[idx] = vga_entry(' ', color);
    }
}

void vga_putchar(char c)
{
    u16 idx = cur_y * VGA_WIDTH + cur_x;

    switch (c) {
    case '\n': 
        cur_x = 0;
        cur_y++;
        break;

    case '\b':
        if (cur_x == 0 && cur_y > 0) {
            cur_y--;
            cur_x = VGA_WIDTH;
        }
        cur_x--;
        idx = cur_y * VGA_WIDTH + cur_x;
        vga[idx] = vga_entry(' ', color);
        break;

    default:
        vga[idx] = vga_entry(c, color);
        cur_x++;
        break;
    }

    if (cur_x >= VGA_WIDTH) {
        cur_x = 0;
        cur_y++;
    }

    if (cur_y >= VGA_HEIGHT) {
        vga_scroll();
        cur_y = VGA_HEIGHT - 1;
    }

    cursor_update(cur_x, cur_y);
}

void vga_print(const char* s)
{
    for (u32 i = 0; s[i]; i++) {
        vga_putchar(s[i]);
    }
}

void vga_puthex(u32 hex)
{
    const char* hexnums = "0123456789ABCDEF";
    char buf[9];
    buf[8] = '\0';

    for (int i = 7; i >= 0; i--) {
        buf[i] = hexnums[hex & 0xF];
        hex >>= 4;
    }

    for (int i = 0; i < 8; i++) {
        vga_putchar(buf[i]);
    }
}

void vga_putnum(int n)
{
    if (n < 0) {
        vga_putchar('-');
        n = -n;
    }

    if (n / 10) {
        vga_putnum(n / 10);
    }

    vga_putchar((n % 10) + '0');
}

void vga_printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        switch (*fmt) {
        case '%':
            fmt++;
            switch (*fmt) {
            case 'c': 
                vga_putchar(va_arg(args, int));
                break;

            case 'S':
            case 's': 
                vga_print(va_arg(args, char*));
                break;

            case 'D':
            case 'd':
            case 'I':
            case 'i':
                vga_putnum(va_arg(args, int));
                break;

            case 'X':
            case 'x':
                vga_puthex(va_arg(args, u32));
                break;
            }
            break;

        default:
            vga_putchar(*fmt);
            break;
        }

        fmt++;
    }

    va_end(args);
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
