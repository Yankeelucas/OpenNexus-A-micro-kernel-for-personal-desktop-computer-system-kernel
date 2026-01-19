; SPDX-License-Identifier: GPL-3.0-or-later
; boot.asm - 系统引导程序
; 功能：从BIOS加载内核到内存并跳转到保护模式

[BITS 16]
[ORG 0x7C00]

; 引导扇区签名
boot_start:
    jmp short start
    nop

; BIOS参数块 (BPB)
bpb:
    times 90-($-$$) db 0

start:
    ; 设置段寄存器
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; 显示启动消息
    mov si, msg_loading
    call print_string

    ; 加载内核到内存
    mov ax, 0x1000      ; 加载到 ES:BX = 0x1000:0x0000
    mov es, ax
    xor bx, bx

    mov ah, 0x02        ; BIOS读磁盘功能
    mov al, 32          ; 读取扇区数 (16KB)
    mov ch, 0           ; 柱面0
    mov cl, 2           ; 从扇区2开始
    mov dh, 0           ; 磁头0
    mov dl, 0x80        ; 第一硬盘
    int 0x13
    jc disk_error

    ; 进入保护模式
    cli
    lgdt [gdt_descriptor]
    
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    
    jmp CODE_SEG:init_pm

[BITS 32]
init_pm:
    ; 设置保护模式段寄存器
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000    ; 设置栈指针

    ; 跳转到内核入口
    jmp 0x10000         ; 内核加载地址

; 字符串打印函数 (实模式)
print_string:
    pusha
    mov ah, 0x0E
.print_char:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .print_char
.done:
    popa
    ret

disk_error:
    mov si, msg_disk_error
    call print_string
    hlt

; 数据区
msg_loading db "Loading Microkernel...", 0x0D, 0x0A, 0
msg_disk_error db "Disk read error!", 0x0D, 0x0A, 0

; GDT定义
gdt_start:
gdt_null:
    dd 0x0
    dd 0x0
gdt_code:
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0
gdt_data:
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; 填充引导扇区
times 510-($-$$) db 0
dw 0xAA55