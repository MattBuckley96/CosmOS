#pragma once

#include "types.h"
#include "console.h"
#include "util.h"
#include "memory.h"

#include "cpu/io.h"
#include "cpu/idt.h"

#include "drivers/keyboard.h"
#include "drivers/timer.h"
#include "drivers/ata.h"

#include "shell/shell.h"

#pragma pack(1)
typedef struct boot_info {
    u32 mmap_addr;
    u32 mmap_entries;
} boot_info_t;
#pragma pack()

extern u8* kernel_start;
extern u8* kernel_end;

void panic(const u8* msg);
