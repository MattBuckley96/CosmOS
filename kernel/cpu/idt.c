#include "idt.h"
#include "pic.h"
#include "gdt.h"
#include "io.h"

///////////////////////////////////////////////

#define IDT_ENTRIES 256

///////////////////////////////////////////////

struct IdtEntry
{
    u16 isr_lo;
    u16 segment;
    u8  reserved;
    u8  flags;
    u16 isr_hi;
} PACKED;

struct Idtr
{
    u16 limit;
    u32 base;
} PACKED;

///////////////////////////////////////////////

static struct IdtEntry idt[IDT_ENTRIES];
static struct Idtr idtr;

///////////////////////////////////////////////

static void isr_default(void)
{
    asm volatile(
        "pusha\n"
        "mov $0, %eax\n"
        "popa\n"
        "iret\n"
    );
}

///////////////////////////////////////////////

void idt_set_descriptor(u8 vector, void* isr, u8 flags)
{
    struct IdtEntry* descriptor = &idt[vector];

    descriptor->isr_lo   = (u32)(uptr)isr & 0xFFFF;
    descriptor->segment  = GDT_KERNEL_CODE_OFFSET;
    descriptor->flags    = flags;
    descriptor->isr_hi   = (u32)(uptr)isr >> 16;
    descriptor->reserved = 0;
}

void idt_init(void)
{
    idtr.base = (u32)(uptr)&idt[0];
    idtr.limit = (u16)sizeof(struct IdtEntry) * IDT_ENTRIES - 1;

    for (u8 i = 0; i < 32; i++)
    {
        idt_set_descriptor(i, isr_default, 0x8E);
    }

    pic_remap();

    // mask all IRQs for now
    outb(PIC_MASTER_DATA, 0xFF);
    outb(PIC_SLAVE_DATA,  0xFF);

    asm volatile("lidt %0" : : "m"(idtr));
    asm volatile("sti");
}
