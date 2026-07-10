#include "memory.h"
#include "console.h"
#include "util.h"

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

            kprintf("Addr: %X\n", addr);
            kprintf("Size: %X\n", size);
        }
    }

    kprintf("\n");
}
