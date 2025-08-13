# === File paths ===
KERNEL_SRC = src/kernel.c src/helpers.c src/terminal.c src/kprintf.c src/gdt.c src/idt.c src/irq.c src/isrs.c src/kbdus.c

# === Build paths ===
KERNEL_OBJ = build/kernel.o build/helpers.o build/terminal.o build/kprintf.o build/gdt.o build/idt.o build/irq.o build/isrs.o build/kbdus.o

BOOT_SRC   = src/boot.s
START_SRC  = src/start.asm
LINKER     = src/linker.ld

BOOT_OBJ   = build/boot.o
START_OBJ  = build/start.o
KERNEL_BIN = build/kernel.bin
ISO_IMAGE  = build/kernel.iso

# === Compiler and assembler paths ===
CROSS_GCC  = /home/obeaj/opt/cross/bin/i686-elf-gcc
CROSS_AS   = /home/obeaj/opt/cross/bin/i686-elf-as

# === Default target ===
all: $(ISO_IMAGE)

# === Compile C files ===
build/%.o: src/%.c
	mkdir -p build
	$(CROSS_GCC) -c $< -o $@ -std=gnu99 -ffreestanding -O2 -Wall -Wextra -fno-builtin -fno-stack-protector -nostdlib -nodefaultlibs

# === Compile boot.s ===
$(BOOT_OBJ): $(BOOT_SRC)
	mkdir -p build
	$(CROSS_AS) $< -o $@

# === Compile start.asm ===
$(START_OBJ): $(START_SRC)
	mkdir -p build
	nasm -f elf32 $< -o $@

# === Link the kernel binary ===
$(KERNEL_BIN): $(BOOT_OBJ) $(KERNEL_OBJ) $(START_OBJ)
	$(CROSS_GCC) -T $(LINKER) -o $@ -ffreestanding -O2 -nostdlib -nodefaultlibs $^ -lgcc

# === Build ISO image with GRUB ===
$(ISO_IMAGE): $(KERNEL_BIN)
	mkdir -p isodir/boot/grub
	cp $< isodir/boot/kernel.bin
	cp boot/grub/grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o $@ isodir

# === Run the kernel with QEMU ===
run: $(ISO_IMAGE)
	qemu-system-i386 -cdrom $(ISO_IMAGE)

# === Clean build files ===
clean:
	rm -rf build/*.o build/*.bin isodir

.PHONY: all clean run