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

typedef struct tss_entry {
    u32 prev_tss;
    u32 esp0;
    u32 ss0;
    u32 esp1;
    u32 ss1;
    u32 esp2;
    u32 ss2;
    u32 cr3;
    u32 eip;
    u32 eflags;
    u32 eax;
    u32 ecx;
    u32 edx;
    u32 ebx;
    u32 esp;
    u32 ebp;
    u32 esi;
    u32 edi;
    u32 es;
    u32 cs;
    u32 ss;
    u32 ds;
    u32 fs;
    u32 gs;
    u32 ldt;    
    u32 trap;
    u32 iomap_base;
} PACKED tss_entry_t;

void gdt_init(void);
void gdt_set_gate(u32 num, u32 base, u32 limit, u8 access, u8 gran);
void tss_write(u32 num, u16 ss0, u32 esp0);