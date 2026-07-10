#pragma once

#include "types.h"

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
