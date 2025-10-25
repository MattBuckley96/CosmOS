#include "types.h"
#include "gdt.h"

///////////////////////////////////////////////

#define GDT_ENTRIES 5

///////////////////////////////////////////////

struct GdtEntry
{
    u16 limit_lo;
    u16 base_lo;
    u8  base_mid;
    u8  access;
    u8  granularity;
    u8  base_hi;
} PACKED; 

struct Gdtr
{
    u16 limit;
    u32 base;
} PACKED;

///////////////////////////////////////////////

static struct GdtEntry gdt[GDT_ENTRIES];
static struct Gdtr gdtr;

///////////////////////////////////////////////

static inline void gdt_flush(struct Gdtr* gdtr)
{
    asm volatile(
        "lgdt (%0)\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        "ljmp $0x08, $1f\n"  // code segment selector (GDT entry 1)
        "1:\n"
        : 
        : "r" (gdtr)
        : "memory", "ax"
    );
}

static void gdt_set_gate(u8 num, u32 base, u32 limit, u8 access, u8 gran)
{
    gdt[num].base_lo     =  (base & 0xFFFF);
    gdt[num].base_mid    =  (base >> 16) & 0xFF;
    gdt[num].base_hi     =  (base >> 24) & 0xFF;
    gdt[num].limit_lo    =  (limit & 0xFFFF);
    gdt[num].granularity =  (limit >> 16) & 0x0F;
    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access      =  access;
}

///////////////////////////////////////////////

void gdt_init(void)
{
    gdtr.limit = (u16)sizeof(struct GdtEntry) * GDT_ENTRIES - 1;
    gdtr.base  = (u32)(uptr)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFF, GDT_KERNEL_CODE, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFF, GDT_KERNEL_DATA, 0xCF);
    gdt_set_gate(3, 0, 0xFFFFF, GDT_USER_CODE,   0xCF);
    gdt_set_gate(4, 0, 0xFFFFF, GDT_USER_DATA,   0xCF);

    gdt_flush(&gdtr);
}
