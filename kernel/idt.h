#pragma once

#include "types.h"

typedef struct idt_entry {
    u16 base_lo;
    u16 sel;
    u8 zero;
    u8 flags;
    u16 base_hi;
} PACKED idt_entry_t;

typedef struct idtr {
    u16 limit;
    u32 base;
} PACKED idtr_t;

typedef struct idt_regs {
    u32 cr2;
    u32 ds;
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 int_no, err_code;
    u32 eip, csm, eflags, useresp, ss;
} PACKED idt_regs_t;

typedef void (*irq_handler_t)(idt_regs_t* regs);

void idt_init(void);
void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags);
void irq_install(u8 irq, irq_handler_t handler);
void irq_uninstall(u8 irq);

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void isr128();
extern void isr177();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();