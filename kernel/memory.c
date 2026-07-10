#include "memory.h"
#include "kernel.h"

u32 memory = 0;
u32 free = 0;

void memory_init(mmap_t* mmap) {
    kprintf("%F[%Fmemory%F]%F\n",
        VGA_WHITE, VGA_LIGHT_GREEN, VGA_WHITE, VGA_GRAY);

    for (u32 i = 0; i < mmap->entry_count; i++) {
        if (mmap->entries[i].type == 1) {
            u64 addr = mmap->entries[i].addr;
            u64 size = mmap->entries[i].size;

            u64 end = addr + size;

            // still in 32 bit mode so ignore above 4 GiB
            if (addr >= 0x100000000 || end > 0x100000000) {
                continue;
            }

            if (addr == 0x100000) {
                u32 kernel_size = (u32)&kernel_end - (u32)&kernel_start;
                memory = (addr + kernel_size);

                free = (end - kernel_size);
            }
        }
    }

    kprintf("first address at: %X\n", memory);
    kprintf("mem free: %u MiB\n", (free / 1024 / 1024));
    kprintf("\n");
}
