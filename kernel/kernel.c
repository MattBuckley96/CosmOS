#include "vga.h"
#include "vga.c"

void ksplash(void)
{
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_print("Welcome to the ");

    vga_set_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK);
    vga_print("Cosm");

    vga_set_color(VGA_COLOR_LIGHT_YELLOW, VGA_COLOR_BLACK);
    vga_print("OS\n");
}

void kmain(void)
{
    vga_init();
    ksplash();
}