#ifndef VGA_H
#define VGA_H

#define VGA_MEMORY 0xB8000
#define VGA_WIDTH  80
#define VGA_HEIGHT 80

void vga_init(void);
void vga_clear(void);
void vga_putc(char c);
void vga_puts(const char* s);

#endif
