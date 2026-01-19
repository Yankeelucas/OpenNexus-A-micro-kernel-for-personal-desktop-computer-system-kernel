; SPDX-License-Identifier: GPL-3.0-or-later
/* linker.ld - 内核链接脚本 */

ENTRY(start)

SECTIONS {
    /* 内核加载地址：1MB */
    . = 1M;
    
    /* 多引导头必须在最前面 */
    .multiboot : {
        *(.multiboot)
    }
    
    /* 文本段 */
    .text : ALIGN(4K) {
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
    }
    
    /* 数据段 */
    .data : ALIGN(4K) {
        *(.data)
        *(.data.*)
    }
    
    /* BSS段 */
    .bss : ALIGN(4K) {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }
    
    /* 内核结束地址 */
    kernel_end = .;
    
    /* 用户空间开始地址 */
    user_start = 4M;
}