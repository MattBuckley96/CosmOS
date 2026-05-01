#include "keyboard.h"
#include "io.h"
#include "vga.h"

bool shift;
bool caps;

static const u8 lowercase[] = {
     0,    27, '1', '2',  '3',  '4', '5',  '6',
    '7',  '8', '9', '0',  '-',  '=', '\b', '\t',
    'q',  'w', 'e', 'r',  't',  'y', 'u',  'i',
    'o',  'p', '[', ']',  '\n',  0,  'a',  's',
    'd',  'f', 'g', 'h',  'j',  'k', 'l',  ';',
    '\'', '`',  0,  '\\', 'z',  'x', 'c',  'v',
    'b',  'n', 'm', ',',  '.',  '/',  0,   '*',
     0,   ' ',
};

static const u8 uppercase[] = {
     0,    27, '!', '@',  '#',  '$', '%',  '^',
    '&',  '*', '(', ')',  '_',  '+', '\b', '\t',
    'Q',  'W', 'E', 'R',  'T',  'Y', 'U',  'I',
    'O',  'P', '{', '}',  '\n',  0,  'A',  'S',
    'D',  'F', 'G', 'H',  'J',  'K', 'L',  ':',
    '"',  '~',  0,  '|',  'Z',  'X', 'C',  'V',
    'B',  'N', 'M', '<',  '>',  '?',  0,   '*',
     0,   ' ',
};

void keyboard_init(void)
{
    shift = false;
    caps = false;
    irq_install(1, &keyboard_irq);
}

void keyboard_irq(idt_regs_t* regs)
{
    (void)regs;

    u8 scan = inb(0x60) & 0x7F;
    u8 press = inb(0x60) & 0x80;

    if (scan >= sizeof(lowercase)) {
        return;
    }

    switch (scan) {
    case 1:
    case 29:
    case 56:
    case 59:
    case 60:
    case 61:
    case 62:
    case 63:
    case 64:
    case 65:
    case 66:
    case 67:
    case 68:
    case 87:
    case 88:
        break;

    case 42: // shift
        if (press == 0) {
            shift = true;
        } else {
            shift = false;
        }
        break;

    case 58:
        if (press == 0) {
            caps= !caps;
        }
        break;

    default:
        if (press == 0) {
            if (shift || caps) {
                vga_printf("%c", uppercase[scan]);
            } else {
                vga_printf("%c", lowercase[scan]);
            }
        }
        break;
    }
}