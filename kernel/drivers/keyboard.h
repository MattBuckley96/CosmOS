#pragma once

#include "cpu/idt.h"

void keyboard_init(void);
void keyboard_irq(idt_regs_t* regs);
u8 keyboard_getchar(void);
