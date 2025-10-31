#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "drivers/vga.h"
#include "drivers/keyboard.h"
#include "drivers/ata.h"
#include "printf.h"
#include "shell/shell.h"

///////////////////////////////////////////////

static void klog(const char* msg)
{
    vga_puts("[ ", VGA_WHITE);
    vga_puts("OK", VGA_LIGHT_GREEN);
    vga_puts(" ] ", VGA_WHITE);
    vga_puts(msg, VGA_LIGHT_GRAY);
    vga_putc('\n', VGA_LIGHT_GRAY);
}

static void panic(const char* msg)
{
    vga_puts("[ ", VGA_WHITE);
    vga_puts("PANIC", VGA_LIGHT_RED);
    vga_puts(" ] ", VGA_WHITE);
    vga_puts(msg, VGA_LIGHT_GRAY);
    vga_putc('\n', VGA_LIGHT_GRAY);
}

static void print_splash(void)
{
    vga_puts("Welcome to the ", VGA_WHITE);
    vga_puts("Cosm", VGA_LIGHT_MAGENTA);
    vga_puts("OS\n\n", VGA_LIGHT_YELLOW);
}

///////////////////////////////////////////////

void kmain(void)
{
    vga_init();
    vga_clear();
    print_splash();

    klog("Initializing GDT...");
    gdt_init();

    klog("Initializing IDT...");
    idt_init();

    klog("Initializing Keyboard...");
    keyboard_init();

    klog("Initializing Disk...");
    if (!ata_init())
    {
        panic("ATA Initialization Failed!");
    }

    printf("\n");

    shell_main();

    while (1)
    {
        asm volatile("hlt");
    }
}
