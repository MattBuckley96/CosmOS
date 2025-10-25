#ifndef IDT_H
#define IDT_H

#include "types.h"

void idt_init(void);
void idt_set_descriptor(u8 vector, void* isr, u8 flags);

#endif
