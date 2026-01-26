// SPDX-License-Identifier: GPL-3.0-or-later
#include <kernel.h>
#include <mm/pmm.h>
#include <proc/process.h>
#include <proc/scheduler.h>
#include <drivers/vga.h>
#include <drivers/keyboard.h>
#include <lib/printf.h>

// 全局内核状态
struct kernel_state kernel_state = {
    .state = SYSTEM_BOOTING,
    .uptime_ticks = 0,
    .total_memory = 0,
    .free_memory = 0,
    .process_count = 0
};

// 早期初始化
static void early_init(void) {
    // 初始化VGA文本模式
    vga_init();
    
    // 显示启动信息
    vga_clear();
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts("\n=== Microkernel Lite ===\n");
    vga_puts("Version: " KERNEL_VERSION "\n");
    vga_puts("Built: " __DATE__ " " __TIME__ "\n\n");
}

// 内存初始化
static void memory_init(void) {
    vga_puts("Initializing memory... ");
    
    // 初始化物理内存管理
    pmm_init();
    
    // 获取内存信息
    kernel_state.total_memory = pmm_get_total();
    kernel_state.free_memory = pmm_get_free();
    
    vga_puts("OK\n");
    vga_printf("  Total: %u KB, Free: %u KB\n", 
               kernel_state.total_memory / 1024,
               kernel_state.free_memory / 1024);
}

// 进程管理初始化
static void process_init(void) {
    vga_puts("Initializing process manager... ");
    
    // 初始化进程管理
    process_manager_init();
    
    // 创建空闲进程
    process_create("idle", 0, NULL);
    
    // 创建初始进程
    process_create("init", 10, NULL);
    
    kernel_state.process_count = 2;
    vga_puts("OK\n");
}

// 设备驱动初始化
static void drivers_init(void) {
    vga_puts("Initializing drivers... ");
    
    // 初始化键盘驱动
    keyboard_init();
    
    vga_puts("OK\n");
}

// 调度器初始化
static void scheduler_init(void) {
    vga_puts("Initializing scheduler... ");
    scheduler_setup();
    vga_puts("OK\n");
}

// GUI子系统检查（不初始化）
static void gui_check(void) {
    vga_puts("GUI subsystem: ");
    vga_puts("Interface only (no implementation)\n");
    vga_puts("  Load gui.ko module to enable graphics\n");
}

// 内核主函数
void kernel_main(void) {
    // 阶段1：基础初始化
    early_init();
    
    // 阶段2：内存管理
    memory_init();
    
    // 阶段3：进程管理
    process_init();
    
    // 阶段4：设备驱动
    drivers_init();
    
    // 阶段5：调度器
    scheduler_init();
    
    // 阶段6：GUI接口检查
    gui_check();
    
    // 更新系统状态
    kernel_state.state = SYSTEM_RUNNING;
    
    // 显示系统信息
    vga_puts("\n=== System Ready ===\n");
    vga_printf("Processes: %u\n", kernel_state.process_count);
    vga_printf("Uptime: %u ticks\n", kernel_state.uptime_ticks);
    vga_puts("Mode: Text-only (no GUI)\n");
    vga_puts("Type 'help' for available commands\n\n");
    
    // 进入调度循环
    scheduler_loop();
}

// 系统时钟中断处理
void system_tick(void) {
    kernel_state.uptime_ticks++;
}

// 内核恐慌处理
void panic(const char* msg) {
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    vga_puts("\n*** KERNEL PANIC ***\n");
    vga_puts(msg);
    vga_puts("\nSystem halted.\n");
    
    kernel_state.state = SYSTEM_PANIC;
    
    // 禁用中断并挂起
    asm volatile("cli");
    while(1) asm volatile("hlt");
}

// 格式化输出
void kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vga_vprintf(fmt, args);
    va_end(args);
}