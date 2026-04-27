CC = i686-elf-gcc
CFLAGS = -ffreestanding -fno-builtin -Wall -Wextra -g -O2

.PHONY: boot kernel image clean

all: image

boot:
	nasm -f elf32 kernel/boot.s -o boot.o

kernel:
	$(CC) $(CFLAGS) -c kernel/kernel.c -o kernel.o

image: clean boot kernel
	mkdir -p build/boot/grub
	ld -m elf_i386 -nostdlib -T linker.ld -o kernel.bin boot.o kernel.o  
	mv kernel.bin build/boot/kernel
	cp grub.cfg build/boot/grub/grub.cfg
	grub-mkrescue -o build/kernel.iso build/
	rm *.o

run: image
	qemu-system-i386 build/kernel.iso

clean:
	rm -rf build