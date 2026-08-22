TARGET_PREFIX ?= i386-elf-
CC := $(TARGET_PREFIX)gcc
LD := $(TARGET_PREFIX)ld

CFLAGS := -ffreestanding -fno-builtin -nostdlib -m32
CFLAGS += -O2 -Wall -g
CFLAGS += -I. -Ikernel
CFLAGS += -Wno-pointer-sign

LDFLAGS := --oformat binary
LDFLAGS += -T kernel.ld -nostdlib
LDFLAGS += -Map=kernel.map

C_SRCS := $(shell find kernel -name '*.c')
ASM_SRCS := $(shell find kernel -name '*.asm')
C_OBJS := $(patsubst %.c, %.c.o, $(C_SRCS))
ASM_OBJS := $(patsubst %.asm, %.asm.o, $(ASM_SRCS))
OBJS := $(C_OBJS) $(ASM_OBJS)

.PHONY: all run debug clean

all: kernel.img

boot.bin:
	nasm -f bin boot/boot.asm -o boot.bin

%.c.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.asm.o: %.asm
	nasm -f elf $< -o $@

test.bin:
	# O2 needed bc stupid GOT gets bypassed bc optimizations
	$(CC) -m32 -fpie -O2 -fno-plt -fvisibility=hidden -nostdlib -ffreestanding -c test/test.c -o test.o
	$(LD) -m elf_i386 -Ttext 0x0 --oformat binary --defsym=_GLOBAL_OFFSET_TABLE_=0 --entry=main test.o -o test.bin
	objdump -D -b binary -m i386 test.bin

kernel.bin: boot.bin $(OBJS) test.bin
	nasm -f elf boot/entry.asm -o entry.o
	$(LD) $(LDFLAGS) entry.o $(OBJS) -o entry.bin
	rm -f $(OBJS) *.o
	cat boot.bin entry.bin > kernel.bin
	rm -f entry.bin boot.bin 
	dd if=/dev/null of=kernel.bin bs=512 count=0 seek=42
	cat test.bin >> kernel.bin
	rm -f test.bin

kernel.img: kernel.bin
	dd if=kernel.bin of=kernel.img
	dd if=/dev/null of=kernel.img bs=1 count=0 seek=10M

run: kernel.img
	qemu-system-i386 -hda kernel.img -monitor stdio -m 512

debug: kernel.img
	qemu-system-i386 -hda kernel.img -monitor stdio -s -S -m 512

clean:
	rm -f *.o *.bin *.img *.map
