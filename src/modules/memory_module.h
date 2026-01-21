// SPDX-License-Identifier: GPL-3.0-or-later
// memory_module.h - 内存管理模块接口定义
#ifndef __MEMORY_MODULE_H__
#define __MEMORY_MODULE_H__

#include "kernel.h"

// ======================
// 内存模块接口定义 (15个函数)
// ======================

// 内存管理模块导出表
typedef struct {
    // 基本函数
    ErrorCode (*init)(void* params);
    ErrorCode (*exit)(void);
    ErrorCode (*query)(ModuleInfo* info);
    
    // 内存池管理接口 (函数1-5)
    MemoryPool* (*mempool_create)(const char* name, MemoryPoolType type, u32 size);
    ErrorCode (*mempool_destroy)(MemoryPool* pool);
    ErrorCode (*mempool_resize)(MemoryPool* pool, u32 new_size);
    MemoryPool* (*mempool_find)(const char* name);
    void (*mempool_stats)(MemoryPool* pool);
    
    // 分配/释放接口 (函数6-10)
    void* (*alloc)(u32 size, MemoryPool* pool);
    void* (*alloc_aligned)(u32 size, u32 alignment, MemoryPool* pool);
    ErrorCode (*free)(void* ptr);
    ErrorCode (*free_pool)(MemoryPool* pool);  // 释放整个池
    u32 (*get_size)(void* ptr);
    
    // 高级功能接口 (函数11-15)
    ErrorCode (*enable_gc)(bool enable);       // 启用/禁用垃圾回收
    ErrorCode (*defragment)(MemoryPool* pool); // 内存碎片整理
    ErrorCode (*set_pool_limit)(MemoryPool* pool, u32 limit);
    ErrorCode (*lock_pool)(MemoryPool* pool);  // 锁定内存池（防止卸载）
    ErrorCode (*unlock_pool)(MemoryPool* pool);
    
    // 模块私有数据
    void* private_data;
} MemoryModuleExports;

// ======================
// 实时内存释放功能接口
// ======================

// 引用计数对象头
typedef struct {
    u32 ref_count;
    u32 size;
    u32 magic;
    void (*destructor)(void* obj);
} RefCountHeader;

// 内存垃圾回收器接口
typedef struct {
    // 垃圾回收器初始化
    ErrorCode (*gc_init)(u32 heap_size);
    
    // 对象管理
    void* (*gc_alloc)(u32 size);
    void (*gc_retain)(void* obj);
    void (*gc_release)(void* obj);
    u32 (*gc_get_refcount)(void* obj);
    
    // 垃圾回收控制
    ErrorCode (*gc_collect)(void);
    ErrorCode (*gc_enable)(bool enable);
    void (*gc_stats)(void);
    
    // 高级功能
    ErrorCode (*gc_set_threshold)(u32 threshold);
    ErrorCode (*gc_register_finalizer)(void* obj, void (*finalizer)(void*));
} GarbageCollector;

// ======================
// 内存池命令接口
// ======================

// 内存池命令结构
typedef struct {
    char command[32];
    char pool_name[32];
    u32 param1;
    u32 param2;
    u32 result;
} MemoryPoolCommand;

// 命令处理器
typedef ErrorCode (*MemoryCommandHandler)(MemoryPoolCommand* cmd);

// 内存池管理器接口
typedef struct {
    // 命令处理
    ErrorCode (*process_command)(const char* cmd_line);
    
    // 池管理
    ErrorCode (*load_pool)(const char* name, const char* config);
    ErrorCode (*unload_pool)(const char* name);
    ErrorCode (*save_pool)(const char* name, const char* filename);
    ErrorCode (*restore_pool)(const char* filename);
    
    // 状态查询
    void (*list_pools)(void);
    void (*pool_info)(const char* name);
    
    // 性能监控
    void (*start_monitor)(const char* name);
    void (*stop_monitor)(const char* name);
    void (*get_stats)(const char* name, void* stats_buffer);
} MemoryPoolManager;

// ======================
// 内存管理模块的配置参数
// ======================

typedef struct {
    // 基本配置
    u32 default_pool_size;
    u32 small_block_size;
    u32 medium_block_size;
    u32 large_block_size;
    
    // 垃圾回收配置
    bool gc_enabled;
    u32 gc_threshold;
    u32 gc_interval;
    
    // 性能优化
    bool use_cache;
    u32 cache_size;
    
    // 调试选项
    bool debug_enabled;
    bool track_allocations;
} MemoryConfig;

// ======================
// 公共API函数声明
// ======================

// 模块注册函数（每个内存模块必须实现）
ErrorCode memory_module_register(MemoryModuleExports* exports);

// 内存系统初始化（由内核调用）
ErrorCode memory_system_init(MemoryConfig* config);

// 工具函数
void* memory_alloc_refcounted(u32 size, void (*destructor)(void*));
void memory_retain(void* obj);
void memory_release(void* obj);

// 命令接口
ErrorCode memory_execute_command(const char* command);

#endif // __MEMORY_MODULE_H__
