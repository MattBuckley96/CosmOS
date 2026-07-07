#pragma once

#include "types.h"

#define CONSOLE_WIDTH 80
#define CONSOLE_HEIGHT 25

typedef enum vga_color {
	VGA_BLACK = 0,
	VGA_BLUE = 1,
	VGA_GREEN = 2,
	VGA_CYAN = 3,
	VGA_RED = 4,
	VGA_MAGENTA = 5,
	VGA_YELLOW = 6,
	VGA_GRAY = 7,
	VGA_DARK_GRAY = 8,
	VGA_LIGHT_BLUE = 9,
	VGA_LIGHT_GREEN = 10,
	VGA_LIGHT_CYAN = 11,
	VGA_LIGHT_RED = 12,
	VGA_LIGHT_MAGENTA = 13,
	VGA_LIGHT_YELLOW = 14,
	VGA_WHITE = 15,
} vga_color_t;

inline static u8 vga_color(u8 fg, u8 bg) {
    return (bg << 4) | fg;
}

inline static u16 console_entry(u8 c, u8 color) {
    return (u16)c | (u16)color << 8;
}

void console_init(void);
void console_set_color(u8 fg, u8 bg);
void console_set_fg(u8 fg);
void console_set_bg(u8 bg);
void console_clear(void);
void console_putchar(u8 c);
void console_print(const u8* str);
void console_putnum(s32 num, u8 base, bool is_signed, bool is_uppercase);
