bits 32
extern kmain

global _start
_start:
    call kmain

    jmp $

times 512-($-$$) db 0
