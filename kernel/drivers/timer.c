#include "timer.h"
#include "cpu/io.h"

u32 ticks;

void timer_init(u32 freq) {
    ticks = 0;
    irq_install(0, &timer_irq);

    // 1.1931816666 MHz
    // 1193182 Hz
    u32 divisor = 1193182 / freq;

    outb(0x43, 0x36);
    outb(0x40, (u8)(divisor & 0xFF));
    outb(0x40, (u8)((divisor >> 8) & 0xFF));
}

void timer_irq(idt_regs_t* regs) {
    (void)regs;

    ticks++;
}

u32 get_ticks(void) {
    return ticks;
}
