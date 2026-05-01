#pragma once

#include "idt.h"

void keyboard_init(void);
void keyboard_irq(idt_regs_t* regs);