#include "types.h"
#include "keyboard.h"

#include "cpu/io.h"
#include "cpu/pic.h"
#include "cpu/idt.h"

///////////////////////////////////////////////

#define PORT_DATA    0x60
#define PORT_STATUS  0x64
#define PORT_COMMAND 0x64

#define LEFT_SHIFT_PRESSED  0x2A
#define LEFT_SHIFT_RELEASED 0xAA

#define BUFFER_SIZE 128

///////////////////////////////////////////////

struct Keyboard
{
    char buf[BUFFER_SIZE];
    u8 head;
    u8 tail;
    bool shift;
};

///////////////////////////////////////////////

static struct Keyboard keyboard;

static const char ascii_table[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=','\b', // 0–13
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,   // 14–28
    'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\','z',   // 29–43
    'x','c','v','b','n','m',',','.','/',0,'*',0,' ',0               // 44–57
};

static const char ascii_table_upper[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+','\b', // 0–13
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',0,            // 14–28
    'A','S','D','F','G','H','J','K','L',':','"','~',0,'|','Z',              // 29–43
    'X','C','V','B','N','M','<','>','?',0,'*',0,' ',0                        // 44–57
};

///////////////////////////////////////////////

void keyboard_isr_handle(void)
{
    u8 scan = inb(PORT_DATA);

    const char* table = ascii_table;
    char c;

    switch (scan)
    {
    case LEFT_SHIFT_PRESSED:
        keyboard.shift = true;
        break;

    case LEFT_SHIFT_RELEASED:
        keyboard.shift = false;
        break;
    
    default:
        if (keyboard.shift)
        {
            table = ascii_table_upper;
        }

        if (scan >= 128)
        {
            break;
        }

        c = table[scan];

        if (c)
        {
            keyboard.buf[keyboard.head] = c;
            keyboard.head = (keyboard.head + 1) % BUFFER_SIZE;
        }
        break;
    }

    // 33 is keyboard irq
    pic_send_eoi(33);
}

///////////////////////////////////////////////

NAKED void keyboard_isr(void)
{
    asm volatile(
        "pusha\n" 
        "call keyboard_isr_handle\n"
        "popa\n"
        "iret\n"
    );
}

void keyboard_init(void)
{
    idt_set_descriptor(33, keyboard_isr, 0x8E);

    keyboard = (struct Keyboard){ 0 };
}

char keyboard_getc(void)
{
    if (keyboard.head == keyboard.tail)
    {
        return 0;
    }

    char c = keyboard.buf[keyboard.tail];
    keyboard.tail = (keyboard.tail + 1) % BUFFER_SIZE;

    return c;
}
