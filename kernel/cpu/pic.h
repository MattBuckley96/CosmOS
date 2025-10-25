#ifndef PIC_H
#define PIC_H

#include "types.h"

#define PIC_MASTER_COMMAND 0x20
#define PIC_MASTER_DATA    0x21
#define PIC_SLAVE_COMMAND  0xA0
#define PIC_SLAVE_DATA     0xA1

#define ICW1_INIT          0x11
#define ICW4_8086          0x01
#define PIC_EOI            0x20

void pic_remap(void);
void pic_send_eoi(u8 irq);

#endif
