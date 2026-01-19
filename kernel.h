// SPDX-License-Identifier: GPL-3.0-or-later
// kernel.h - 内核核心数据结构和常量定义
#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// ======================
// 基础类型定义
// ======================
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

// ======================
// 核心常量定义
// ======================
#define NULL            0
#define true           1
#define false          0

#define KB             1024
#define MB             (1024 * KB)
#define GB             (1024 * MB)

#define MAX_PROCESSES  256
#define MAX_MODULES    64
#define MAX_MEMPOOLS   16
#define MAX_IPC_QUEUES 128

#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10
#define USER_CODE_SELECTOR   0x18
#define USER_DATA_SELECTOR   0x20

// ======================
// 错误代码定义
// ======================
typedef enum {
    ERR_SUCCESS = 0,
    ERR_GENERIC = -1,
    ERR_NO_MEMORY = -2,
    ERR_INVALID_ARG = -3,
    ERR_NOT_FOUND = -4,
    ERR_EXISTS = -5,
    ERR_PERMISSION = -6,
    ERR_BUSY = -7,
    ERR_TIMEOUT = -8,
    ERR_NOT_IMPLEMENTED = -9,
    ERR_MODULE_NOT_LOADED = -10,
    ERR_INVALID_MODULE = -11,
    ERR_MEMPOOL_FULL = -12,
    ERR_MEMPOOL_EMPTY = -13,
    ERR_IPC_QUEUE_FULL = -14
} ErrorCode;

// ======================
// 模块系统相关定义
// ======================
typedef enum {
    MODULE_STATUS_UNLOADED = 0,
    MODULE_STATUS_LOADING,
    MODULE_STATUS_ACTIVE,
    MODULE_STATUS_SUSPENDED,
    MODULE_STATUS_ERROR
} ModuleStatus;

typedef enum {
    MODULE_TYPE_CORE = 0,      // 核心模块（不可卸载）
    MODULE_TYPE_MEMORY,        // 内存管理模块
    MODULE_TYPE_FILESYSTEM,    // 文件系统模块
    MODULE_TYPE_DEVICE,        // 设备驱动模块
    MODULE_TYPE_NETWORK,       // 网络模块
    MODULE_TYPE_SECURITY,      // 安全模块
    MODULE_TYPE_UTILITY        // 工具模块
} ModuleType;

// 模块信息结构
typedef struct {
    char name[32];
    char version[16];
    char author[32];
    char description[128];
    ModuleType type;
    u32 api_version;
    u32 flags;
} ModuleInfo;

// 模块导出表（模块必须实现的接口）
typedef struct {
    // 必需函数
    ErrorCode (*init)(void* params);
    ErrorCode (*exit)(void);
    ErrorCode (*query)(ModuleInfo* info);
    
    // 模块特定函数指针数组（最多15个）
    void* functions[15];
    
    // 模块私有数据
    void* private_data;
} ModuleExportTable;

// ======================
// 内存管理相关定义
// ======================
typedef enum {
    MEM_TYPE_KERNEL = 0,
    MEM_TYPE_USER,
    MEM_TYPE_DMA,
    MEM_TYPE_CACHE,
    MEM_TYPE_RESERVED
} MemoryType;

typedef enum {
    MEM_POOL_DEFAULT = 0,
    MEM_POOL_SMALL,      // 小对象池（< 1KB）
    MEM_POOL_MEDIUM,     // 中等对象池（1KB - 4KB）
    MEM_POOL_LARGE,      // 大对象池（> 4KB）
    MEM_POOL_SPECIAL     // 特殊用途池
} MemoryPoolType;

// 内存池描述符
typedef struct {
    u32 id;
    char name[32];
    MemoryPoolType type;
    u32 base_address;
    u32 size;
    u32 used;
    u32 block_size;
    u32 flags;
    
    // 统计信息
    u32 allocations;
    u32 frees;
    u32 peak_usage;
    
    // 链表
    struct MemoryPool* next;
} MemoryPool;

// ======================
// 进程管理相关定义
// ======================
typedef enum {
    PROC_STATE_NEW = 0,
    PROC_STATE_READY,
    PROC_STATE_RUNNING,
    PROC_STATE_BLOCKED,
    PROC_STATE_SUSPENDED,
    PROC_STATE_ZOMBIE,
    PROC_STATE_DEAD
} ProcessState;

typedef struct {
    u32 pid;
    char name[64];
    ProcessState state;
    u32 priority;
    u32 entry_point;
    u32 stack_base;
    u32 stack_size;
    u32 heap_base;
    u32 heap_size;
    
    // 上下文保存区域
    u32 registers[16];
    
    // 进程资源
    u32* page_directory;
    MemoryPool* mempool;
    
    // 统计信息
    u64 cpu_time;
    u32 memory_used;
    
    // 链表
    struct Process* next;
    struct Process* prev;
} Process;

// ======================
// IPC系统定义
// ======================
typedef enum {
    IPC_TYPE_MESSAGE = 0,
    IPC_TYPE_SHARED_MEMORY,
    IPC_TYPE_SEMAPHORE,
    IPC_TYPE_EVENT,
    IPC_TYPE_PIPE
} IPCType;

typedef struct {
    u32 id;
    IPCType type;
    u32 sender_pid;
    u32 receiver_pid;
    u32 size;
    void* data;
    u32 flags;
    u64 timestamp;
    
    struct IPCMessage* next;
} IPCMessage;

// ======================
// 系统调用定义
// ======================
typedef enum {
    SYS_NOP = 0,
    SYS_EXIT,
    SYS_FORK,
    SYS_EXEC,
    SYS_WAIT,
    SYS_GETPID,
    SYS_GETTIME,
    
    // 内存管理
    SYS_ALLOC,
    SYS_FREE,
    SYS_MEMINFO,
    SYS_MPOOL_CREATE,
    SYS_MPOOL_DESTROY,
    
    // 进程通信
    SYS_IPC_SEND,
    SYS_IPC_RECEIVE,
    SYS_IPC_QUEUE_CREATE,
    SYS_IPC_QUEUE_DELETE,
    
    // 模块管理
    SYS_MODULE_LOAD,
    SYS_MODULE_UNLOAD,
    SYS_MODULE_QUERY,
    SYS_MODULE_CALL,
    
    // 文件系统
    SYS_OPEN,
    SYS_CLOSE,
    SYS_READ,
    SYS_WRITE,
    SYS_SEEK,
    SYS_STAT,
    
    SYS_MAX
} SyscallNumber;

// 系统调用参数结构
typedef struct {
    u32 syscall_num;
    u32 arg1;
    u32 arg2;
    u32 arg3;
    u32 arg4;
    u32 arg5;
    u32 result;
} SyscallParams;

// ======================
// 全局函数声明
// ======================

// 内核初始化
void kernel_early_init(void);
void kernel_main(void);

// 中断处理
void isr_install(void);
void irq_install(void);

// 系统调用
void syscall_handler(SyscallParams* params);

// 实用函数
void kprintf(const char* fmt, ...);
void panic(const char* msg);
void memset(void* dest, u8 value, u32 size);
void memcpy(void* dest, const void* src, u32 size);
int memcmp(const void* s1, const void* s2, u32 n);

#endif // __KERNEL_H__