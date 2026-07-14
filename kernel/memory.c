#include "memory.h"
#include "kernel.h"

u32 start;
u32 max;
u32 pos;

// malloc gives full 4K pages back
// TODO: implement proper heap
void* malloc(u32 size) {
    void* ptr = 0;
    u32 aligned = ALIGN_UP(size, PAGE_SIZE);

    if ((pos + aligned) > max) {
        panic("out of memory!");
    }

    ptr = (void*)pos;
    pos += aligned;

    return ptr;
}

void memory_init(mmap_t* mmap) {
    for (u32 i = 0; i < mmap->entry_count; i++) {
        if (mmap->entries[i].type == 1) {
            u64 addr = mmap->entries[i].addr;
            u64 size = mmap->entries[i].size;

            u64 end = addr + size;

            // still in 32 bit mode so ignore above 4 GiB
            if (addr >= 0x100000000 || end > 0x100000000) {
                continue;
            }

            // skip regions below kernel
            // TODO: may revisit later
            if (addr < 0x100000) {
                continue;
            }

            u32 kernel_size = (u32)&kernel_end - (u32)&kernel_start;
            kernel_size = ALIGN_UP(kernel_size, PAGE_SIZE);

            // only stores the first region beyond the kernel
            // TODO: combine different regions
            start = (kernel_size + addr);
            pos = start;
            max = end;
            break;
        }
    }
}

u32 get_total_memory(void) {
    return (max - start);
}

u32 get_used_memory(void) {
    return (pos - start);
}
