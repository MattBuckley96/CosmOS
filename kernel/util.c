#include <stdarg.h>
#include "util.h"
#include "console.h"

inline void* memset(void* addr, u8 val, u32 size) {
    u8* buf = (u8*)addr;
    for (u32 i = 0; i < size; i++) {
        buf[i] = val;
    }
    return buf;
}

inline void* mem_zero(void* addr, u32 size) {
    return memset(addr, 0, size);
}

void* memcpy(void* dest, const void* src, u32 size) {
    u8* d = (u8*)dest;
    u8* s = (u8*)src;

    for (u32 i = 0; i < size; i++) {
        d[i] = s[i];
    }
    return dest;
}

s32 strcmp(const u8* a, const u8* b) {
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return (s32)(*a) - (s32)(*b);
}

u8* strcpy(u8* dest, const u8* src) {
    for (u32 i = 0; src[i]; i++) {
        dest[i] = src[i];
    }
    return dest;
}

void kprintf(const u8* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        switch (*fmt) {
        case '%':
            fmt++; // eat the %
            switch (*fmt) {
            case 'c':
                console_putchar(va_arg(args, u32));
                break;

            case 'S':
            case 's': {
                u8* str = va_arg(args, u8*);
                if (str) {
                    console_print(str);
                } else {
                    console_print("(null)");
                }
                break;
            }

            case 'D':
            case 'd':
            case 'I':
            case 'i':
                console_putnum(va_arg(args, s32), 10, true, false);
                break;

            case 'p':
            case 'x':
                console_putnum(va_arg(args, s32), 16, false, false);
                break;

            case 'X':
                console_putnum(va_arg(args, s32), 16, false, true);
                break;

            case 'U':
            case 'u':
                console_putnum(va_arg(args, s32), 10, false, false);
                break;

            case 'b':
                console_putnum(va_arg(args, s32), 2, false, false);
                break;

            case 'F':
                console_set_fg(va_arg(args, u32));
                break;

            case 'B':
                console_set_bg(va_arg(args, u32));
                break;
            }
            break;

        default:
            console_putchar(*fmt);
            break;
        }

        fmt++;
    }

    va_end(args);
}
