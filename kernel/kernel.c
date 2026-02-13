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
    vga_puts(" OK ", VGA_LIGHT_GREEN);
    vga_puts(" ] ", VGA_WHITE);
    vga_puts(msg, VGA_LIGHT_GRAY);
    vga_putc('\n', VGA_LIGHT_GRAY);
}

static void kfail(const char* msg)
{
    vga_puts("[ ", VGA_WHITE);
    vga_puts("FAIL", VGA_LIGHT_RED);
    vga_puts(" ] ", VGA_WHITE);
    vga_puts(msg, VGA_LIGHT_GRAY);
    vga_putc('\n', VGA_LIGHT_GRAY);
}

static void panic(const char* msg)
{
    vga_puts("[ ", VGA_WHITE);
    vga_puts("PANIC", VGA_RED);
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

static void pause(int duration)
{
    for (int i = 0; i < 1000000; i++)
        for (int j = 0; j < 100 * duration; j++)
            asm volatile("nop");
}

///////////////////////////////////////////////

void kmain(void)
{
    vga_init();
    vga_clear();

    printf("And then... there was ");
    // annoying
    // pause(1);
    vga_puts("light!\n\n", VGA_WHITE);

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
        kfail("No file system");
        klog("Making file system...");
        fs_create();
    }

    printf("\n");
    print_splash();

    printf("sizeof dentry: %i\n", sizeof(struct Dentry));

    shell_main();

    while (1)
    {
        asm volatile("hlt");
    }

    panic("How did we get here?");
}
