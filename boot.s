bits 32

section .multiboot
    align 4
    dd 0x1BADB002
    dd 0x00000000
    dd -(0x1BADB002 + 0x00000000)

section .bss
align 16
stack_bottom:
resb 16384 
stack_top:

section .text
global start
extern kmain

start:
    cli
    mov esp, stack_top
    call kmain

    cli
halt:
    hlt
    jmp halt