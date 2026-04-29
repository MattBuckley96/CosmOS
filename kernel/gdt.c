#include "gdt.h"
#include "util.h"

extern void gdt_flush(u32 addr);
extern void tss_flush(void);

gdt_entry_t gdt_entries[6];
gdtr_t gdtr;
tss_entry_t tss_entry;

void gdt_init(void)
{
    gdtr.limit = (sizeof(gdt_entry_t) * 6) - 1;
    gdtr.base = (u32)&gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0); // null
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // kernel code
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // kernel data
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // user code
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // user data
    tss_write(5, 0x10, 0x0);

    gdt_flush((u32)&gdtr);
    tss_flush();
}

void gdt_set_gate(u32 num, u32 base, u32 limit, u8 access, u8 gran)
{
    gdt_entries[num].base_hi = base & 0xFFFF;
    gdt_entries[num].base_mid = (base >> 16) & 0xFF;
    gdt_entries[num].base_hi = (base >> 24) & 0xFF;

    gdt_entries[num].limit = limit & 0xFFFF;
    gdt_entries[num].flags = (limit >> 16) & 0x0F;
    gdt_entries[num].flags |= gran & 0xF0;

    gdt_entries[num].access = access;
}

void tss_write(u32 num, u16 ss0, u32 esp0)
{
    u32 base = (u32)&tss_entry;
    u32 limit = base + sizeof(tss_entry);

    gdt_set_gate(num, base, limit, 0xE9, 0x00);
    memset(&tss_entry, 0, sizeof(tss_entry));

    tss_entry.ss0 = ss0;
    tss_entry.esp0 = esp0;

    tss_entry.cs = 0x08 | 0x3;
    tss_entry.ss = tss_entry.ds = tss_entry.es = 
    tss_entry.fs = tss_entry.gs = 0x10 | 0x3;
}