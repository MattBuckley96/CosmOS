CROSS ?= i686-elf-
CC = $(CROSS)gcc
AS = $(CROSS)gcc
LD = $(CROSS)ld

################################################

CFLAGS  = -m32 -ffreestanding -fno-builtin -g -Ikernel -Ikernel/libc
ASFLAGS = -m32 -ffreestanding
LDFLAGS = -T linker.ld -nostdlib

################################################

BUILD_DIR  = build
BOOT_DIR   = boot
KERNEL_DIR = kernel
GRUB_DIR   = grub
OBJ_DIR    = $(BUILD_DIR)/obj

################################################

C_SRCS := $(shell find $(KERNEL_DIR) -name '*.c')
S_SRCS := $(BOOT_DIR)/boot.s

C_OBJS := $(patsubst %.c, $(OBJ_DIR)/%.c.o, $(C_SRCS))
S_OBJS := $(patsubst %.s, $(OBJ_DIR)/%.s.o, $(S_SRCS))
OBJS   := $(C_OBJS) $(S_OBJS)

################################################

KERNEL_BIN = $(BUILD_DIR)/kernel.bin
ISO_DIR    = $(BUILD_DIR)/iso
ISO        = $(ISO_DIR)/kernel.iso
IMG_DIR    = $(BUILD_DIR)/img
IMG        = $(IMG_DIR)/disk.img

################################################

.PHONY: all iso run clean disk
all: $(KERNEL_BIN) iso disk

################################################

$(OBJ_DIR)/%.c.o: %.c | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.s.o: %.s | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

################################################

$(KERNEL_BIN): $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

################################################

iso: $(KERNEL_BIN)
	rm -rf $(ISO_DIR)
	mkdir -p $(ISO_DIR)/boot/grub
	mv $(KERNEL_BIN) $(ISO_DIR)/boot/kernel.bin
	cp $(GRUB_DIR)/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) $(ISO_DIR)

disk: $(IMG_DIR)
	@if [ ! -f $(IMG) ]; then \
		qemu-img create $(IMG) 10M; \
	fi 

################################################

run: iso disk
	qemu-system-i386 -cdrom $(ISO) -m 512 -hda $(IMG)

################################################

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(IMG_DIR):
	mkdir -p $(IMG_DIR)

clean:
	rm -rf $(BUILD_DIR)
