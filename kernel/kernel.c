#include "vga.h"
#include "gdt.h"

void kmain(void)
{
    vga_init();

    // gdt init
    vga_print("Initializing GDT...\n");
    gdt_init();

    // splash
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("\nWelcome to the ");

    vga_set_color(VGA_LIGHT_MAGENTA, VGA_BLACK);
    vga_print("Cosm");

    vga_set_color(VGA_LIGHT_YELLOW, VGA_BLACK);
    vga_print("OS\n");

    vga_set_color(VGA_GRAY, VGA_BLACK);
}