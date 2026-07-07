#include "console.h"
#include "cpu/io.h"

volatile u16* screen = (u16*)0xb8000;
u16 row;
u16 col;
u8 color;

static void cursor_enable(u8 start, u8 end) {
    outb(0x3d4, 0x0a);
	outb(0x3d5, (inb(0x3d5) & 0xc0) | start);

	outb(0x3d4, 0x0b);
	outb(0x3d5, (inb(0x3d5) & 0xe0) | end);
}

static void cursor_update(u8 col, u8 row) {
    u16 pos = row * CONSOLE_WIDTH + col;

    outb(0x3d4, 0x0f);
    outb(0x3d5, (u8)(pos & 0xff));
    outb(0x3d4, 0x0e);
    outb(0x3d5, (u8)((pos >> 8) & 0xff));
}

static void scroll(void) {
    for (u16 y = 0; y < CONSOLE_HEIGHT; y++) {
        for (u16 x = 0; x < CONSOLE_WIDTH; x++) {
            u16 index = y * CONSOLE_WIDTH + x;
            screen[(y - 1) * CONSOLE_WIDTH + x] = screen[index];
        }
    }

    row = CONSOLE_HEIGHT - 1;
    for (u16 x = 0; x < CONSOLE_WIDTH; x++) {
        u16 index = row * CONSOLE_WIDTH + x;
        screen[index] = console_entry(' ', color);
    }
}

void console_init(void) {
    cursor_enable(14, 15);

    console_set_color(VGA_GRAY, VGA_BLACK);
    console_clear();
}

void console_set_color(u8 fg, u8 bg) {
    color = vga_color(fg, bg);
}

void console_set_fg(u8 fg) {
    color = (color & ~0x0f) | (fg & 0x0f);
}

void console_set_bg(u8 bg) {
    color = (color & ~0xf0) | (bg & 0xf0);
}

void console_clear(void) {
    for (u32 i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT; i++) {
        screen[i] = console_entry(' ', color);
    }

    row = 0;
    col = 0;
}

void console_putchar(u8 c) {
    u16 index = row * CONSOLE_WIDTH + col;

    switch (c) {
    case '\n':
        col = 0;
        row++;
        break;
    
    case '\b':
        if (col == 0 && row > 0) {
            row--;
            col = CONSOLE_WIDTH;
        }
        col--;
        index = row * CONSOLE_WIDTH + col;
        screen[index] = console_entry(' ', color);
        break;

    default:
        screen[index] = console_entry(c, color);
        col++;
        break;
    }

    if (col >= CONSOLE_WIDTH) {
        col = 0;
        row++;
    }

    if (row >= CONSOLE_HEIGHT) {
        scroll();
    }

    cursor_update(col, row);
}

void console_print(const u8* str) {
    while (*str) {
        console_putchar(*str);
        str++;
    }
}

static const u8* lowercase = "0123456789abcdef";
static const u8* uppercase = "0123456789ABCDEF";

void console_putnum(s32 num, u8 base, bool is_signed, bool is_uppercase) {
    const u8* digits = lowercase;

    // prob wont need more than 12, u32 max is 10 digits
    u8 buf[12];
    u8* ptr = &buf[11];
    *ptr = '\0';

    if (is_uppercase) {
        digits = uppercase;
    }

    u32 n;
    if (is_signed && num < 0) {
        n = (u32)(-num);
    } else {
        n = (u32)num;
    }

    do {
        ptr--;
        *ptr = digits[n % base];
        n /= base;
    } while (n > 0);

    if (is_signed && num < 0) {
        ptr--;
        *ptr = '-';
    }

    if (base == 16) {
        ptr--;
        *ptr = 'x';
        ptr--;
        *ptr = '0';
    }

    while (*ptr) {
        console_putchar(*ptr);
        ptr++;
    }
}
