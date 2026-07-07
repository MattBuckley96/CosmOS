bits 16
org 0x7C00

KERNEL_LOC equ 0x1000
KERNEL_ADDR equ 0x100000
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

start16:
    mov [boot_drive], dl   

    cli
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov bp, 0x7C00
    mov sp, bp
    sti

    ; set video mode
    mov ax, 0x0003
    int 0x10

    ; load kernel
    mov ah, 0x02
    mov al, 25
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    mov bx, KERNEL_LOC
    int 0x13
    jc disk_err


check_a20:
    in al, 0x92
    test al, 2
    jnz .done
    or al, 2
    and al, 0xFE
    out 0x92, al
.done:
    ; load gdt
    cli
    lgdt [gdt_desc]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp CODE_SEG:start32

disk_err:
    mov si, disk_err_msg
    call print
    jmp $

print:
    mov ah, 0x0E
.loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .loop
.done:
    ret

boot_drive:
    db 0

gdt_start:
    gdt_null:
        dd 0
        dd 0
    gdt_code:
        dw 0xFFFF
        dw 0x0
        db 0x0
        db 0b10011010
        db 0b11001111
        db 0x0
    gdt_data:
        dw 0xFFFF
        dw 0x0
        db 0x0
        db 0b10010010
        db 0b11001111
        db 0x0
gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

disk_err_msg: db "[boot]: disk error!", 0

bits 32
start32:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x90000
    mov esp, ebp

    ; HACK: don't leave this
    cld
    mov esi, KERNEL_LOC
    mov edi, KERNEL_ADDR
    mov ecx, 10000
    rep movsd

    jmp KERNEL_ADDR

times 510-($-$$) db 0
dw 0xAA55
