#include "cpu/gdt.h"
#include "drivers/vga.h"

void print_splash(void)
{
    vga_puts("Welcome to the ", VGA_WHITE);
    vga_puts("Cosm", VGA_LIGHT_MAGENTA);
    vga_puts("OS\n", VGA_LIGHT_YELLOW);
}

void kmain(void)
{
    vga_init();
    vga_clear();
    print_splash();

    gdt_init();

    while (1)
    {
        asm volatile("hlt");
    }
}
