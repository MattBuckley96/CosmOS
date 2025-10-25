#ifndef GDT_H
#define GDT_H

#define GDT_KERNEL_CODE        0x9A
#define GDT_KERNEL_DATA        0x92
#define GDT_USER_CODE          0xFA
#define GDT_USER_DATA          0xF2

#define GDT_KERNEL_CODE_OFFSET 0x0008

void gdt_init(void);

#endif
