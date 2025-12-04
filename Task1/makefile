# 要链接进内核的所有目标文件
OBJECTS = source/loader.o \
	source/kmain.o \
	drivers/io.o \
	drivers/frame_buffer.o \
	drivers/hardware_interrupt_enabler.o \
	drivers/interrupt_asm.o \
	drivers/interrupts.o \
	drivers/keyboard.o \
	drivers/terminal.o \
	drivers/pic.o \
	drivers/input_buffer.o


CC      = gcc
CFLAGS  = -I. -m32 -std=c99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
	-nostartfiles -nodefaultlibs -Wall -Wextra -c
LDFLAGS = -T source/link.ld -melf_i386
AS      = nasm
ASFLAGS = -f elf

.PHONY: all clean run run-nographic run-curses

all: kernel.elf

# 链接生成内核
kernel.elf: $(OBJECTS)
	ld $(LDFLAGS) $(OBJECTS) -o kernel.elf

# 生成可启动 ISO
os.iso: kernel.elf
	cp kernel.elf iso/boot/kernel.elf
	genisoimage -R \
		-b boot/grub/stage2_eltorito \
		-no-emul-boot \
		-boot-load-size 4 \
		-A os \
		-input-charset utf8 \
		-quiet \
		-boot-info-table \
		-o os.iso \
		iso

# 使用图形界面运行（会弹出 QEMU 窗口）
run: os.iso
	qemu-system-i386 -boot d -cdrom os.iso -m 32

# 无图形界面运行（在终端中）
run-nographic: os.iso
	qemu-system-i386 -nographic -boot d -cdrom os.iso -m 32

# curses 模式运行，带 monitor / serial
run-curses: os.iso
	qemu-system-i386 -display curses \
		-boot d \
		-cdrom os.iso \
		-m 32

# 通用编译规则：C 源文件
source/%.o: source/%.c
	$(CC) $(CFLAGS) $< -o $@

drivers/%.o: drivers/%.c
	$(CC) $(CFLAGS) $< -o $@

# 通用编译规则：汇编源文件（nasm）
source/%.o: source/%.s
	$(AS) $(ASFLAGS) $< -o $@

drivers/%.o: drivers/%.s
	$(AS) $(ASFLAGS) $< -o $@

# 清理
clean:
	rm -rf *.o source/*.o drivers/*.o kernel.elf os.iso iso/boot/kernel.elf
