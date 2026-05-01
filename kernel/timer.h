#pragma once

#include "idt.h"
#include "types.h"

void timer_init(u32 freq);
void timer_irq(idt_regs_t* regs);