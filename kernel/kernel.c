#include "kernel.h"

void panic(const u8* msg) {
    kprintf("%F[%Fpanic%F]:%F ", VGA_WHITE, VGA_LIGHT_RED, VGA_WHITE, VGA_LIGHT_RED);
    kprintf("%s", msg);
    asm volatile("hlt");
    for (;;);
}

void kmain(boot_info_t* boot_info) {
    console_init();

    kprintf("Let there be %Flight!%F\n\n", VGA_WHITE, VGA_GRAY);

    idt_init();
    keyboard_init();
    timer_init(500);
    ata_init();

    mmap_t mmap = {
        .entry_count = boot_info->mmap_entries,
        .entries = (mmap_entry_t*)boot_info->mmap_addr,
    };
    memory_init(&mmap);

    u32* x = malloc(sizeof(u32));

    shell();

    panic("How did we get here?");
}
