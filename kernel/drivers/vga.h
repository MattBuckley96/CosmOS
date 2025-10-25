#ifndef VGA_H
#define VGA_H

#include "types.h"

#define VGA_MEMORY        0xB8000
#define VGA_WIDTH         80
#define VGA_HEIGHT        80

#define VGA_BLACK         0x0
#define VGA_BLUE          0x1
#define VGA_GREEN         0x2
#define VGA_CYAN          0x3
#define VGA_RED           0x4
#define VGA_MAGENTA       0x5
#define VGA_YELLOW        0x6
#define VGA_LIGHT_GRAY    0x7

#define VGA_DARK_GRAY     0x8
#define VGA_LIGHT_BLUE    0x9
#define VGA_LIGHT_GREEN   0xA
#define VGA_LIGHT_CYAN    0xB
#define VGA_LIGHT_RED     0xC
#define VGA_LIGHT_MAGENTA 0xD
#define VGA_LIGHT_YELLOW  0xE
#define VGA_WHITE         0xF

///////////////////////////////////////////////

void vga_init(void);
void vga_clear(void);
void vga_putc(char c, u8 col);
void vga_puts(const char* s, u8 col);

#endif
