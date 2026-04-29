#include "gdt.h"

extern void gdt_flush(u32 addr);

gdt_entry_t gdt_entries[5];
gdtr_t gdtr;

void gdt_init(void)
{
    gdtr.limit = (sizeof(gdt_entry_t) * 5) - 1;
    gdtr.base = (u32)&gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0); // null
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // kernel code
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // kernel data
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // user code
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // user data

    gdt_flush((u32)&gdtr);
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