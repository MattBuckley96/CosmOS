#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "drivers/vga.h"

void klog(const char* msg)
{
    vga_puts("[ ", VGA_WHITE);
    vga_puts("OK", VGA_LIGHT_GREEN);
    vga_puts(" ] ", VGA_WHITE);
    vga_puts(msg, VGA_LIGHT_GRAY);
    vga_putc('\n', VGA_BLACK);
}

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

    klog("Initializing GDT...");
    gdt_init();

    klog("Initializing IDT...");
    idt_init();

    while (1)
    {
        asm volatile("hlt");
    }
}
