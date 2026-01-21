; SPDX-License-Identifier: GPL-3.0-or-later
# Makefile - 微内核构建系统

# 工具定义
AS = nasm
CC = i686-elf-gcc
LD = i686-elf-ld
OBJCOPY = i686-elf-objcopy
QEMU = qemu-system-i386

# 编译标志
ASFLAGS = -f elf32
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -I. \
         -m32 -march=i686 -mgeneral-regs-only -nostdlib \
         -fno-stack-protector -fno-pic -fno-builtin
LDFLAGS = -T linker.ld -nostdlib -lgcc

# 源文件列表
BOOT_SRC = boot.asm
KERNEL_SRCS = kernel.c process.c memory.c ipc.c syscall.c module.c
HEADERS = kernel.h process.h memory.h ipc.h syscall.h module.h \
          memory_module.h fs_module.h

# 目标文件
OBJS = $(BOOT_SRC:.asm=.o) $(KERNEL_SRCS:.c=.o)

# 默认目标
all: microkernel.iso

# 编译引导程序
boot.o: $(BOOT_SRC)
	$(AS) $(ASFLAGS) $< -o $@

# 编译C文件
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# 链接内核
kernel.bin: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) $(OBJS) -o $@

# 创建ISO镜像
microkernel.iso: kernel.bin grub.cfg
	mkdir -p isodir/boot/grub
	cp kernel.bin isodir/boot/
	cp grub.cfg isodir/boot/grub/
	grub-mkrescue -o $@ isodir

# 在QEMU中运行
run: microkernel.iso
	$(QEMU) -cdrom microkernel.iso -m 16M -serial stdio

# 调试运行
debug: microkernel.iso
	$(QEMU) -cdrom microkernel.iso -m 16M -serial stdio -s -S &
	sleep 1
	i686-elf-gdb -ex "target remote localhost:1234" -ex "symbol-file kernel.bin"

# 清理
clean:
	rm -f $(OBJS) kernel.bin microkernel.iso
	rm -rf isodir

# 创建模块模板
template-module: template.c
	$(CC) $(CFLAGS) -c template.c -o template.o
	$(LD) -r -o template_module.bin template.o

.PHONY: all run debug clean template-module
