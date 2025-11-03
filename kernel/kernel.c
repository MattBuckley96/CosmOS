#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "drivers/vga.h"
#include "drivers/keyboard.h"
#include "drivers/ata.h"
#include "printf.h"
#include "shell/shell.h"
#include "fs/fs.h"

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
    int err = ata_init();
    if (err)
        panic("ATA doesnt work idiot");

    klog("Initializing File System...");
    err = fs_init();
    if (err)
    {
        printf("Making file system...\n");
        shell_mkfs(0, NULL);
    }

    printf("\n");

    // HACK: reading first allows writes????
    // what bullshit is that
    ata_read(0, NULL, 1);

    shell_main();

    while (1)
    {
        asm volatile("hlt");
    }
}
