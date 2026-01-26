# SPDX-License-Identifier: GPL-3.0-or-later

# 工具链配置
CC = i686-elf-gcc
AS = nasm
LD = i686-elf-ld
OBJCOPY = i686-elf-objcopy
QEMU = qemu-system-i386

# 编译标志
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra \
         -Iinclude -Iinclude/drivers -Iinclude/gui \
         -Iinclude/mm -Iinclude/proc \
         -m32 -march=i686 -mgeneral-regs-only \
         -nostdlib -fno-stack-protector -fno-pic -fno-builtin

ASFLAGS = -f elf32
LDFLAGS = -T linker.ld -nostdlib -lgcc

# 源文件
C_SOURCES = $(shell find kernel lib -name "*.c")
ASM_SOURCES = $(shell find kernel -name "*.asm")
OBJS = $(C_SOURCES:.c=.o) $(ASM_SOURCES:.asm=.o)

# 目标文件
KERNEL = kernel.bin
ISO = microkernel.iso

# 默认目标
.PHONY: all clean run debug iso

all: $(ISO)

# 编译规则
%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# 链接内核
$(KERNEL): $(OBJS) linker.ld
	$(LD) $(LDFLAGS) $(OBJS) -o $@
	@echo "Linked: $(KERNEL)"

# 创建ISO镜像
$(ISO): $(KERNEL) scripts/grub.cfg
	@mkdir -p isodir/boot/grub
	cp $(KERNEL) isodir/boot/
	cp scripts/grub.cfg isodir/boot/grub/
	grub-mkrescue -o $@ isodir
	@echo "Created ISO: $@"

# 在QEMU中运行
run: $(ISO)
	$(QEMU) -cdrom $(ISO) -m 16M -serial stdio -no-reboot -no-shutdown

# 调试运行
debug: $(ISO)
	$(QEMU) -cdrom $(ISO) -m 16M -serial stdio -s -S &
	sleep 2
	i686-elf-gdb -ex "target remote localhost:1234" \
                 -ex "symbol-file $(KERNEL)" \
                 -ex "b kernel_main"

# 仅构建内核（无ISO）
kernel: $(KERNEL)

# 清理
clean:
	find . -name "*.o" -type f -delete
	find . -name "*.bin" -type f -delete
	rm -f $(KERNEL) $(ISO)
	rm -rf isodir
	@echo "Build cleaned"

# 帮助信息
help:
	@echo "=== Microkernel Lite Build System ==="
	@echo "Targets:"
	@echo "  all     - Build kernel ISO (default)"
	@echo "  kernel  - Build kernel only"
	@echo "  run     - Run in QEMU"
	@echo "  debug   - Run with GDB debugger"
	@echo "  clean   - Remove build files"
	@echo "  help    - Show this help"
	@echo ""
	@echo "System: Text-only mode, no GUI implementation"
	@echo "License: GPL-3.0-or-later"