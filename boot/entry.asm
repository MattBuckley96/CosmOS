bits 32

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .text
extern kmain
global _start
_start:
    mov esp, stack_top
    mov ebp, esp

    push eax ; boot_info
    call kmain

    jmp $

times 512-($-$$) db 0
