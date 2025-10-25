#include "drivers/vga.h"

void kmain(void)
{
    vga_init();
    vga_clear();
    vga_puts("Welcome to the CosmOS!\n");

    while (1)
    {
        asm volatile("hlt");
    }
}
