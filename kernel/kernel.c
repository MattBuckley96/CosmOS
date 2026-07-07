#include "kernel.h"

void panic(const u8* msg) {
    kprintf("%F[%Fpanic%F]:%F ", VGA_WHITE, VGA_LIGHT_RED, VGA_WHITE, VGA_LIGHT_RED);
    kprintf("%s", msg);
    asm volatile("hlt");
    for (;;);
}

void kmain(void) {
    console_init();

    kprintf("Let there be %Flight!%F\n\n", VGA_WHITE, VGA_GRAY);

    idt_init();
    keyboard_init();

    for (;;) {
        u8 c = keyboard_getchar();
        if (c) {
            kprintf("%c", c);
        }
    }

    panic("How did we get here?");
}
