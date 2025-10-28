#include <stdarg.h>
#include "printf.h"
#include "drivers/vga.h"

static void put_num(int n)
{
    if (n < 0)
    {
        vga_putc('-', VGA_LIGHT_GRAY);
        n = -n;
    }

    if (n / 10)
    {
        put_num(n / 10);
    }
    
    vga_putc((n % 10) + '0', VGA_LIGHT_GRAY);
}

void printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    while (*format)
    {
        switch (*format)
        {
        case '%':
            format++;
            switch (*format)
            {
            case 's':
            case 'S':
                vga_puts(va_arg(args, char*), VGA_LIGHT_GRAY);
                break;
            case 'd':
            case 'i':
                put_num(va_arg(args, int));
                break;
            }
            break;

        default:
            vga_putc(*format, VGA_LIGHT_GRAY);
        }

        format++;
    }

    va_end(args);
}
