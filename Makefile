CC = i686-elf-gcc
LD = i386-elf-ld
CFLAGS = -ffreestanding -fno-builtin -Wall -Wextra -g

.PHONY: boot kernel image clean

all: image

boot:
	nasm -f elf32 kernel/boot.s -o boot.o
	nasm -f elf32 kernel/gdt.s -o gdt.s.o
	nasm -f elf32 kernel/idt.s -o idt.s.o

kernel:
	$(CC) $(CFLAGS) -c kernel/kernel.c -o kernel.o
	$(CC) $(CFLAGS) -c kernel/vga.c -o vga.o
	$(CC) $(CFLAGS) -c kernel/gdt.c -o gdt.o
	$(CC) $(CFLAGS) -c kernel/idt.c -o idt.o
	$(CC) $(CFLAGS) -c kernel/timer.c -o timer.o
	$(CC) $(CFLAGS) -c kernel/keyboard.c -o keyboard.o
	$(CC) $(CFLAGS) -c kernel/util.c -o util.o

image: clean boot kernel
	mkdir -p build/boot/grub
	$(LD) -nostdlib -T linker.ld -o kernel.bin boot.o kernel.o vga.o gdt.o gdt.s.o idt.o idt.s.o timer.o keyboard.o util.o
	mv kernel.bin build/boot/kernel.bin
	cp grub.cfg build/boot/grub/grub.cfg
	grub-mkrescue -o build/kernel.iso build/
	rm *.o

run: image
	qemu-system-i386 build/kernel.iso

clean:
	rm -rf build
	rm -f *.o
