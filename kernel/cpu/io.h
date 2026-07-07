#pragma once

#include "types.h"

inline static void outb(u16 port, u8 val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

inline static u8 inb(u16 port) {
    u8 val;
    asm volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

inline static void outw(u16 port, u16 val) {
    asm volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

inline static u16 inw(u16 port) {
    u16 val;
    asm volatile("inw %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

inline static void insw(u16 port, void* addr, u32 count) {
    asm volatile(
        "rep insw"
        : "+D"(addr), "+c"(count)
        : "d"(port)
        : "memory");
}
