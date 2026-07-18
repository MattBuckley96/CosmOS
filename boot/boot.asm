bits 16
org 0x7C00

KERNEL_LOC equ 0x1000
KERNEL_ADDR equ 0x100000
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; TODO: this may end up overridden by kernel 
; if it gets too big.
; also its dangerously close to 0x7C00
BOOT_INFO equ 0x7000
MMAP_ADDR equ 0x8000

start16:
    mov [boot_drive], dl

    cli
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov sp, 0x6000
    sti

    mov si, boot_msg
    call print

set_video:
    mov ax, 0x0003
    int 0x10

load_kernel:
    mov ah, 0x02
    mov al, 33
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    mov bx, KERNEL_LOC
    int 0x13
    jc .error
    jmp .done
.error:
    mov si, disk_err_msg
    call print
    hlt
    jmp $
.done:

check_a20:
    in al, 0x92
    test al, 2
    jnz .done
    or al, 2
    and al, 0xFE
    out 0x92, al
.done:

detect_memory: 
    mov di, MMAP_ADDR
    xor ebx, ebx
    mov word [mmap_entry_count], 0
.next:
    mov eax, 0xE820
    mov edx, 0x534D4150
    mov ecx, 24
    mov [es:di + 20], dword 1 ; force ACPI 3.X entry
    int 0x15
    jc .error

    cmp eax, 0x534D4150
    jne .error

    add di, 24
    inc word [mmap_entry_count]
    test ebx, ebx
    jne .next

.done:
    mov dword [BOOT_INFO], MMAP_ADDR
    movzx eax, word [mmap_entry_count]
    mov dword [BOOT_INFO + 4], eax
    jmp load_gdt

.error:
    mov si, mmap_err_msg
    call print
    hlt
    jmp $

load_gdt:
    cli
    lgdt [gdt_desc]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp CODE_SEG:start32

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

mmap_entry_count:
    dw 0

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

disk_err_msg: 
    db "[boot]: disk error!", 0

mmap_err_msg:
    db "[boot] memory map error!", 0

boot_msg:
    db "[boot] booting...", 0

bits 32
start32:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

move_kernel:
    ; HACK: don't leave this
    cld
    mov esi, KERNEL_LOC
    mov edi, KERNEL_ADDR
    mov ecx, 40000
    rep movsb

    mov eax, BOOT_INFO
    jmp KERNEL_ADDR

times 510-($-$$) db 0
dw 0xAA55
