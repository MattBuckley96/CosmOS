#pragma once

#include "types.h"

#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

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

static inline u8 vga_color(u8 fg, u8 bg)
{
    return fg | bg << 4;
}

static inline u16 vga_entry(char c, u8 color)
{
    return (u16)c | (u16)color << 8;
}

void vga_init(void);
void vga_set_color(u8 fg, u8 bg);
void vga_scroll(void);
void vga_putchar(char c);
void vga_print(const char* s);

void cursor_enable(u8 start, u8 end);
void cursor_disable(void);
void cursor_update(u8 x, u8 y);