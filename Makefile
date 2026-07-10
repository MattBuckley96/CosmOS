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

all: kernel.bin

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

run: kernel.bin
	qemu-system-i386 -enable-kvm kernel.bin -monitor stdio -m 512

debug: kernel.bin
	qemu-system-i386 kernel.bin -monitor stdio -s -S -m 512

clean:
	rm -f *.o *.bin
