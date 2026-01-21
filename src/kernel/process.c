// SPDX-License-Identifier: GPL-3.0-or-later
// process.c - 进程管理和调度器
#include "kernel.h"
#include "process.h"
#include "memory.h"

// 进程表
static Process* process_table[MAX_PROCESSES];
static u32 process_count = 0;
static u32 next_pid = 1;

// 就绪队列
static Process* ready_queue = NULL;
static Process* blocked_queue = NULL;
static Process* zombie_queue = NULL;

// 进程管理器初始化
ErrorCode process_manager_init(void) {
    kprintf("Initializing process manager...\n");
    
    memset(process_table, 0, sizeof(process_table));
    process_count = 0;
    next_pid = 1;
    
    // 创建空闲进程
    Process* idle = process_create("idle", 0, 0);
    if (!idle) {
        panic("Failed to create idle process");
    }
    
    kprintf("  Process manager ready\n");
    return ERR_SUCCESS;
}

// 创建新进程
Process* process_create(const char* name, u32 priority, u32 entry_point) {
    if (process_count >= MAX_PROCESSES) {
        kprintf("ERROR: Process table full\n");
        return NULL;
    }
    
    // 分配进程结构
    Process* proc = (Process*)memory_alloc(sizeof(Process));
    if (!proc) {
        kprintf("ERROR: Failed to allocate process structure\n");
        return NULL;
    }
    
    // 初始化进程
    memset(proc, 0, sizeof(Process));
    proc->pid = next_pid++;
    strncpy(proc->name, name, sizeof(proc->name) - 1);
    proc->state = PROC_STATE_NEW;
    proc->priority = priority;
    proc->entry_point = entry_point;
    
    // 分配栈空间
    proc->stack_size = 16 * KB;
    proc->stack_base = (u32)memory_alloc(proc->stack_size);
    if (!proc->stack_base) {
        kprintf("ERROR: Failed to allocate stack for process\n");
        memory_free(proc);
        return NULL;
    }
    
    // 分配堆空间
    proc->heap_size = 64 * KB;
    proc->heap_base = (u32)memory_alloc(proc->heap_size);
    if (!proc->heap_base) {
        kprintf("ERROR: Failed to allocate heap for process\n");
        memory_free((void*)proc->stack_base);
        memory_free(proc);
        return NULL;
    }
    
    // 初始化寄存器上下文
    process_init_context(proc);
    
    // 添加到进程表
    for (u32 i = 0; i < MAX_PROCESSES; i++) {
        if (!process_table[i]) {
            process_table[i] = proc;
            break;
        }
    }
    
    process_count++;
    
    // 添加到就绪队列
    process_set_state(proc, PROC_STATE_READY);
    
    kprintf("  Created process: %s (PID: %d)\n", name, proc->pid);
    return proc;
}

// 初始化进程上下文
void process_init_context(Process* proc) {
    // 设置初始栈指针（栈从高地址向低地址生长）
    u32* stack = (u32*)(proc->stack_base + proc->stack_size);
    
    // 在栈上压入初始返回地址（进程退出处理函数）
    stack--;
    *stack = (u32)process_exit_handler;
    
    // 压入初始EFLAGS
    stack--;
    *stack = 0x202;  // 中断启用
    
    // 设置指令指针
    proc->registers[0] = proc->entry_point;  // EIP
    proc->registers[1] = (u32)stack;         // ESP
    
    // 初始化其他寄存器
    for (int i = 2; i < 16; i++) {
        proc->registers[i] = 0;
    }
}

// 设置进程状态
ErrorCode process_set_state(Process* proc, ProcessState new_state) {
    if (!proc) return ERR_INVALID_ARG;
    
    ProcessState old_state = proc->state;
    proc->state = new_state;
    
    // 从原队列移除
    process_remove_from_queue(proc, old_state);
    
    // 添加到新队列
    process_add_to_queue(proc, new_state);
    
    kprintf("  Process %d: %s -> %s\n", 
            proc->pid, 
            process_state_to_string(old_state),
            process_state_to_string(new_state));
    
    return ERR_SUCCESS;
}

// 从队列移除进程
void process_remove_from_queue(Process* proc, ProcessState state) {
    Process** queue = NULL;
    
    switch (state) {
        case PROC_STATE_READY:
            queue = &ready_queue;
            break;
        case PROC_STATE_BLOCKED:
            queue = &blocked_queue;
            break;
        case PROC_STATE_ZOMBIE:
            queue = &zombie_queue;
            break;
        default:
            return;
    }
    
    if (*queue == proc) {
        *queue = proc->next;
        if (*queue) (*queue)->prev = NULL;
    } else {
        if (proc->prev) proc->prev->next = proc->next;
        if (proc->next) proc->next->prev = proc->prev;
    }
    
    proc->prev = NULL;
    proc->next = NULL;
}

// 添加到队列
void process_add_to_queue(Process* proc, ProcessState state) {
    Process** queue = NULL;
    
    switch (state) {
        case PROC_STATE_READY:
            queue = &ready_queue;
            break;
        case PROC_STATE_BLOCKED:
            queue = &blocked_queue;
            break;
        case PROC_STATE_ZOMBIE:
            queue = &zombie_queue;
            break;
        default:
            return;
    }
    
    proc->next = *queue;
    proc->prev = NULL;
    if (*queue) (*queue)->prev = proc;
    *queue = proc;
}

// 进程退出处理
void process_exit_handler(void) {
    Process* current = process_get_current();
    if (!current) return;
    
    kprintf("Process %d (%s) exiting\n", current->pid, current->name);
    
    // 设置为僵尸状态
    process_set_state(current, PROC_STATE_ZOMBIE);
    
    // 切换到其他进程
    scheduler_yield();
    
    // 永不返回
    for(;;);
}

// 回收僵尸进程
void process_reap_zombies(void) {
    Process* curr = zombie_queue;
    while (curr) {
        Process* next = curr->next;
        
        kprintf("Reaping zombie process %d\n", curr->pid);
        
        // 释放进程资源
        if (curr->stack_base) {
            memory_free((void*)curr->stack_base);
        }
        if (curr->heap_base) {
            memory_free((void*)curr->heap_base);
        }
        
        // 从进程表移除
        for (u32 i = 0; i < MAX_PROCESSES; i++) {
            if (process_table[i] == curr) {
                process_table[i] = NULL;
                break;
            }
        }
        
        // 释放进程结构
        memory_free(curr);
        process_count--;
        
        curr = next;
    }
    
    zombie_queue = NULL;
}

// 调度器主循环
void scheduler_loop(void) {
    kprintf("Starting scheduler loop\n");
    
    while (1) {
        // 选择要运行的进程
        Process* next = scheduler_select_next();
        
        if (next && next != process_get_current()) {
            // 上下文切换
            process_switch(process_get_current(), next);
        }
        
        // 执行系统维护任务
        system_maintenance();
    }
}

// 选择下一个要运行的进程
Process* scheduler_select_next(void) {
    // 简单的时间片轮转调度
    static Process* last = NULL;
    
    if (!ready_queue) {
        // 没有就绪进程，返回空闲进程
        return process_find("idle");
    }
    
    if (!last) {
        last = ready_queue;
    }
    
    // 查找下一个就绪进程
    Process* next = last->next;
    while (next && next->state != PROC_STATE_READY) {
        next = next->next;
    }
    
    if (!next) {
        next = ready_queue;
        while (next && next->state != PROC_STATE_READY) {
            next = next->next;
        }
    }
    
    if (next) {
        last = next;
        return next;
    }
    
    // 没有就绪进程，返回空闲进程
    return process_find("idle");
}

// 进程上下文切换
void process_switch(Process* from, Process* to) {
    if (!to) return;
    
    // 保存当前进程上下文
    if (from) {
        process_save_context(from);
        from->state = PROC_STATE_READY;
    }
    
    // 恢复目标进程上下文
    to->state = PROC_STATE_RUNNING;
    process_restore_context(to);
    
    // 更新当前进程指针
    process_set_current(to);
}

// 保存进程上下文（汇编实现）
void process_save_context(Process* proc) {
    asm volatile(
        "movl %%eax, 0(%0)\n"
        "movl %%ebx, 4(%0)\n"
        "movl %%ecx, 8(%0)\n"
        "movl %%edx, 12(%0)\n"
        "movl %%esi, 16(%0)\n"
        "movl %%edi, 20(%0)\n"
        "movl %%ebp, 24(%0)\n"
        "movl %%esp, 28(%0)\n"
        :
        : "r"(proc->registers)
        : "memory"
    );
}

// 恢复进程上下文（汇编实现）
void process_restore_context(Process* proc) {
    asm volatile(
        "movl 0(%0), %%eax\n"
        "movl 4(%0), %%ebx\n"
        "movl 8(%0), %%ecx\n"
        "movl 12(%0), %%edx\n"
        "movl 16(%0), %%esi\n"
        "movl 20(%0), %%edi\n"
        "movl 24(%0), %%ebp\n"
        "movl 28(%0), %%esp\n"
        :
        : "r"(proc->registers)
        : "memory"
    );
    
    // 设置指令指针（通过iret返回）
    asm volatile(
        "pushl $0x10\n"        // 用户数据段选择子
        "pushl 28(%0)\n"       // ESP
        "pushl 0x202\n"        // EFLAGS
        "pushl $0x08\n"        // 用户代码段选择子
        "pushl 0(%0)\n"        // EIP
        "iret\n"
        :
        : "r"(proc)
    );
}

// 查找进程
Process* process_find(const char* name) {
    for (u32 i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] && strcmp(process_table[i]->name, name) == 0) {
            return process_table[i];
        }
    }
    return NULL;
}

Process* process_find_by_pid(u32 pid) {
    for (u32 i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] && process_table[i]->pid == pid) {
            return process_table[i];
        }
    }
    return NULL;
}

// 获取当前进程
Process* process_get_current(void) {
    return current_process;
}

// 设置当前进程
void process_set_current(Process* proc) {
    current_process = proc;
}

// 状态转换为字符串
const char* process_state_to_string(ProcessState state) {
    switch (state) {
        case PROC_STATE_NEW:       return "NEW";
        case PROC_STATE_READY:     return "READY";
        case PROC_STATE_RUNNING:   return "RUNNING";
        case PROC_STATE_BLOCKED:   return "BLOCKED";
        case PROC_STATE_SUSPENDED: return "SUSPENDED";
        case PROC_STATE_ZOMBIE:    return "ZOMBIE";
        case PROC_STATE_DEAD:      return "DEAD";
        default:                   return "UNKNOWN";
    }
}

// 列出所有进程
void process_list_all(void) {
    kprintf("\n=== Processes (%d) ===\n", process_count);
    kprintf("PID   State     Pri Name\n");
    
    for (u32 i = 0; i < MAX_PROCESSES; i++) {
        Process* proc = process_table[i];
        if (proc) {
            kprintf("%-5d %-9s %-3d %s\n", 
                    proc->pid,
                    process_state_to_string(proc->state),
                    proc->priority,
                    proc->name);
        }
    }
}