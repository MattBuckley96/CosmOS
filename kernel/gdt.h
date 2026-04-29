#pragma once

#include "types.h"

typedef struct gdt_entry {
    u16 limit;
    u16 base_lo;
    u8 base_mid;
    u8 access;
    u8 flags;
    u8 base_hi;
} PACKED gdt_entry_t;

typedef struct gdtr {
    u16 limit;
    u32 base;
} PACKED gdtr_t;

void gdt_init(void);
void gdt_set_gate(u32 num, u32 base, u32 limit, u8 access, u8 gran);
