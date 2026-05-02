#include "util.h"

#include "types.h"
#include "vga.h"

void* memset(void* dest, u8 val, u32 size)
{
    u8* temp = (u8*)dest;
    for (u32 i = 0; i < size; i++)
    {
        temp[i] = val;
    }
    return dest;
}

void print_regs(void)
{
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 cs, ds, es, fs, gs, ss;

    asm("mov %%eax, %0" : "=r"(eax));
    asm("mov %%ebx, %0" : "=r"(ebx));
    asm("mov %%ecx, %0" : "=r"(ecx));
    asm("mov %%edx, %0" : "=r"(edx));
    asm("mov %%esp, %0" : "=r"(esp));
    asm("mov %%ebp, %0" : "=r"(ebp));
    asm("mov %%esi, %0" : "=r"(esi));
    asm("mov %%edi, %0" : "=r"(edi));

    asm("mov %%cs, %0" : "=r"(cs));
    asm("mov %%ds, %0" : "=r"(ds));
    asm("mov %%es, %0" : "=r"(es));
    asm("mov %%fs, %0" : "=r"(fs));
    asm("mov %%gs, %0" : "=r"(gs));
    asm("mov %%ss, %0" : "=r"(ss));

    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_printf("registers: \n");
    vga_set_color(VGA_GRAY, VGA_BLACK);

    vga_printf("eax: %x, ", eax);
    vga_printf("ebx: %x, ", ebx);
    vga_printf("ecx: %x\n", ecx);
    vga_printf("edx: %x, ", edx);
    vga_printf("esp: %x, ", esp);
    vga_printf("ebp: %x\n", ebp);
    vga_printf("esi: %x, ", esi);
    vga_printf("edi: %x\n", edi);

    vga_printf("cs: %x, ", cs);
    vga_printf("ds: %x, ", ds);
    vga_printf("es: %x\n", es);
    vga_printf("fs: %x, ", fs);
    vga_printf("gs: %x, ", gs);
    vga_printf("ss: %x\n\n", ss);
}
