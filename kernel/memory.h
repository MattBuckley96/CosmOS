#pragma once

/*
 * memory layout so far:
 *
 * --------------------------------------------------------------------
 * | 0x1000 |  0x7000   |  0x8000    | 0x100000 | 0x200000-0xFFFFFFFF |
 * -------------------------------------------------------------------- 
 *  kernel   boot_info  mmap_entries   kernel          free?
 *
 */

#include "types.h"

#define PAGE_SIZE 4096
#define ALIGN_UP(n, align) (((n) + ((align) - 1)) & ~((align) - 1))

#pragma pack(1)
typedef struct mmap_entry {
    u64 addr;
    u64 size;
    u32 type;
    u32 apci_ext;
} mmap_entry_t;

typedef struct mmap {
    u32 entry_count;
    mmap_entry_t* entries;
} mmap_t;
#pragma pack()

void memory_init(mmap_t* mmap);
void* malloc(u32 size);
u32 get_total_memory(void);
u32 get_used_memory(void);
