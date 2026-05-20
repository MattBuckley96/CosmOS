#include "vga.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "keyboard.h"
#include "util.h"
#include "shell.h"
#include "ata.h"

void kmain(void)
{
    vga_init();

    kprintf("And then... there was ");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    kprintf("light!\n\n");
    vga_set_color(VGA_GRAY, VGA_BLACK);

    gdt_init();
    kprintf("Initialized GDT\n");

    idt_init();
    kprintf("Initialized IDT\n");

    timer_init(500);
    kprintf("Initialized Timer to 500hz\n");

    keyboard_init();
    kprintf("Initialized Keyboard\n");

    ata_init();
    kprintf("Initialized ATA drive\n");

    // splash
    vga_set_color(VGA_WHITE, VGA_BLACK);
    kprintf("\nWelcome to the ");

    vga_set_color(VGA_LIGHT_MAGENTA, VGA_BLACK);
    kprintf("Cosm");

    vga_set_color(VGA_LIGHT_YELLOW, VGA_BLACK);
    kprintf("OS\n\n");

    vga_set_color(VGA_GRAY, VGA_BLACK);

    shell();

    for (;;);
}
