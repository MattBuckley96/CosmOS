TARGET_PREFIX ?= i386-elf-
CC := $(TARGET_PREFIX)gcc 
LD := $(TARGET_PREFIX)ld

CFLAGS := -ffreestanding -fno-builtin -nostdlib -m32
CFLAGS += -O2 -Wall -g
CFLAGS += -I. -Ikernel
CFLAGS += -Wno-pointer-sign

LDFLAGS := --oformat binary
LDFLAGS += -T linker.ld -nostdlib

C_SRCS := $(shell find kernel -name '*.c')
ASM_SRCS := $(shell find kernel -name '*.asm')
C_OBJS := $(patsubst %.c, %.c.o, $(C_SRCS))
ASM_OBJS := $(patsubst %.asm, %.asm.o, $(ASM_SRCS))
OBJS := $(C_OBJS) $(ASM_OBJS)

all: kernel.img

boot.bin:
	nasm -f bin boot/boot.asm -o boot.bin

%.c.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.asm.o: %.asm
	nasm -f elf $< -o $@

kernel.bin: boot.bin $(OBJS)
	nasm -f elf boot/entry.asm -o entry.o
	$(LD) $(LDFLAGS) entry.o $(OBJS) -o entry.bin
	cat boot.bin entry.bin > kernel.bin
	rm -f $(OBJS) *.o
	rm -f boot.bin entry.bin

kernel.img: kernel.bin
	dd if=kernel.bin of=kernel.img
	dd if=/dev/null of=kernel.img bs=1 count=0 seek=10M
	rm -f *.bin

run: kernel.img
	qemu-system-i386 -enable-kvm -hda kernel.img -monitor stdio -m 512

debug: kernel.img
	qemu-system-i386 -hda kernel.img -monitor stdio -s -S -m 512

clean:
	rm -f *.o *.bin *.img
