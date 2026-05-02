#include "vga.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "keyboard.h"
#include "util.h"

void kmain(void)
{
    vga_init();

    print_regs();

    gdt_init();
    vga_print("Initialized GDT\n");

    idt_init();
    vga_print("Initialized IDT\n");

    timer_init(500);
    vga_print("Initialized Timer to 500hz\n");

    keyboard_init();
    vga_print("Initialized Keyboard\n");

    // splash
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("\nWelcome to the ");

    vga_set_color(VGA_LIGHT_MAGENTA, VGA_BLACK);
    vga_print("Cosm");

    vga_set_color(VGA_LIGHT_YELLOW, VGA_BLACK);
    vga_print("OS\n");

    vga_set_color(VGA_GRAY, VGA_BLACK);

    for (;;);
}
