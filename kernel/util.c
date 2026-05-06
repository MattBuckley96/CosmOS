#include <stdarg.h>
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

void kprintf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vga_vprintf(fmt, args);
    va_end(args);
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
    kprintf("registers: \n");
    vga_set_color(VGA_GRAY, VGA_BLACK);

    kprintf("eax: %x, ", eax);
    kprintf("ebx: %x, ", ebx);
    kprintf("ecx: %x\n", ecx);
    kprintf("edx: %x, ", edx);
    kprintf("esp: %x, ", esp);
    kprintf("ebp: %x\n", ebp);
    kprintf("esi: %x, ", esi);
    kprintf("edi: %x\n", edi);

    kprintf("cs: %x, ", cs);
    kprintf("ds: %x, ", ds);
    kprintf("es: %x\n", es);
    kprintf("fs: %x, ", fs);
    kprintf("gs: %x, ", gs);
    kprintf("ss: %x\n\n", ss);
}

int strcmp(const char* a, const char* b)
{
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return (int)(*a) - (int)(*b);
}