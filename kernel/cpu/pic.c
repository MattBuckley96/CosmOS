#include "pic.h"
#include "io.h"

void pic_remap(void) {
    uint8_t master_mask, slave_mask;

    master_mask = inb(PIC_MASTER_DATA);
    slave_mask  = inb(PIC_SLAVE_DATA);

    outb(PIC_MASTER_COMMAND, ICW1_INIT);
    outb(PIC_SLAVE_COMMAND, ICW1_INIT);

    outb(PIC_MASTER_DATA, 0x20);
    outb(PIC_SLAVE_DATA,  0x28);

    outb(PIC_MASTER_DATA, 0x04);
    outb(PIC_SLAVE_DATA,  0x02);

    outb(PIC_MASTER_DATA, ICW4_8086);
    outb(PIC_SLAVE_DATA,  ICW4_8086);

    outb(PIC_MASTER_DATA, master_mask);
    outb(PIC_SLAVE_DATA,  slave_mask);
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC_SLAVE_COMMAND, PIC_EOI);
    }
    outb(PIC_MASTER_COMMAND, PIC_EOI);
}
