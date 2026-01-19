// SPDX-License-Identifier: GPL-3.0-or-later
// memory.c - 基础内存管理器（Bootstrap版本）
#include "kernel.h"
#include "memory.h"

// 内存管理内部结构
typedef struct MemoryBlock {
    u32 size;
    bool free;
    struct MemoryBlock* next;
    struct MemoryBlock* prev;
} MemoryBlock;

// 全局内存信息
static u32 memory_total = 0;
static u32 memory_used = 0;
static u32 memory_free = 0;
static u32 memory_block_count = 0;

// 内存块链表头
static MemoryBlock* memory_list = NULL;

// 内存池数组
static MemoryPool* memory_pools[MAX_MEMPOOLS];
static u32 memory_pool_count = 0;

// 内存管理器初始化
ErrorCode memory_init(void) {
    kprintf("Initializing memory manager...\n");
    
    // 获取内存大小（从BIOS或通过探测）
    memory_total = 16 * MB;  // 假设16MB
    memory_free = memory_total;
    
    kprintf("  Total memory: %d MB\n", memory_total / MB);
    
    // 初始化内核堆
    u32 heap_start = 0x100000;  // 1MB以上
    u32 heap_size = 4 * MB;
    
    memory_list = (MemoryBlock*)heap_start;
    memory_list->size = heap_size - sizeof(MemoryBlock);
    memory_list->free = true;
    memory_list->next = NULL;
    memory_list->prev = NULL;
    
    memory_block_count = 1;
    
    // 初始化内存池数组
    memset(memory_pools, 0, sizeof(memory_pools));
    memory_pool_count = 0;
    
    // 创建默认内存池
    MemoryPool* default_pool = mempool_create("default", MEM_POOL_DEFAULT, 1 * MB);
    if (!default_pool) {
        panic("Failed to create default memory pool");
    }
    
    kprintf("  Memory manager ready\n");
    return ERR_SUCCESS;
}

// 内存分配（首次适应算法）
void* memory_alloc(u32 size) {
    if (size == 0) return NULL;
    
    // 对齐到8字节边界
    size = (size + 7) & ~7;
    
    MemoryBlock* current = memory_list;
    while (current) {
        if (current->free && current->size >= size + sizeof(MemoryBlock)) {
            // 分割内存块
            MemoryBlock* new_block = (MemoryBlock*)((u8*)current + sizeof(MemoryBlock) + size);
            new_block->size = current->size - size - sizeof(MemoryBlock);
            new_block->free = true;
            new_block->next = current->next;
            new_block->prev = current;
            
            if (current->next) {
                current->next->prev = new_block;
            }
            
            current->size = size;
            current->free = false;
            current->next = new_block;
            
            memory_used += size + sizeof(MemoryBlock);
            memory_free -= size + sizeof(MemoryBlock);
            memory_block_count++;
            
            return (void*)((u8*)current + sizeof(MemoryBlock));
        }
        current = current->next;
    }
    
    return NULL;  // 内存不足
}

// 内存释放
ErrorCode memory_free(void* ptr) {
    if (!ptr) return ERR_INVALID_ARG;
    
    MemoryBlock* block = (MemoryBlock*)((u8*)ptr - sizeof(MemoryBlock));
    
    if (block->free) {
        return ERR_GENERIC;  // 重复释放
    }
    
    block->free = true;
    memory_used -= block->size + sizeof(MemoryBlock);
    memory_free += block->size + sizeof(MemoryBlock);
    
    // 合并相邻的空闲块
    memory_coalesce();
    
    return ERR_SUCCESS;
}

// 合并相邻空闲块
void memory_coalesce(void) {
    MemoryBlock* current = memory_list;
    
    while (current && current->next) {
        if (current->free && current->next->free) {
            // 合并块
            current->size += current->next->size + sizeof(MemoryBlock);
            current->next = current->next->next;
            
            if (current->next) {
                current->next->prev = current;
            }
            
            memory_block_count--;
        } else {
            current = current->next;
        }
    }
}

// 创建内存池
MemoryPool* mempool_create(const char* name, MemoryPoolType type, u32 size) {
    if (memory_pool_count >= MAX_MEMPOOLS) {
        kprintf("ERROR: Memory pool limit reached\n");
        return NULL;
    }
    
    // 分配内存池结构
    MemoryPool* pool = (MemoryPool*)memory_alloc(sizeof(MemoryPool));
    if (!pool) {
        kprintf("ERROR: Failed to allocate pool structure\n");
        return NULL;
    }
    
    // 分配池内存
    void* pool_memory = memory_alloc(size);
    if (!pool_memory) {
        kprintf("ERROR: Failed to allocate pool memory\n");
        memory_free(pool);
        return NULL;
    }
    
    // 初始化池
    memset(pool, 0, sizeof(MemoryPool));
    pool->id = memory_pool_count + 1;
    strncpy(pool->name, name, sizeof(pool->name) - 1);
    pool->type = type;
    pool->base_address = (u32)pool_memory;
    pool->size = size;
    pool->block_size = mempool_get_block_size(type);
    pool->flags = 0;
    
    // 添加到池数组
    for (u32 i = 0; i < MAX_MEMPOOLS; i++) {
        if (!memory_pools[i]) {
            memory_pools[i] = pool;
            break;
        }
    }
    
    memory_pool_count++;
    
    kprintf("  Created memory pool: %s (ID: %d, Size: %d KB)\n", 
            name, pool->id, size / KB);
    
    return pool;
}

// 根据类型获取块大小
u32 mempool_get_block_size(MemoryPoolType type) {
    switch (type) {
        case MEM_POOL_SMALL:   return 64;
        case MEM_POOL_MEDIUM:  return 256;
        case MEM_POOL_LARGE:   return 1024;
        case MEM_POOL_SPECIAL: return 4096;
        default:               return 128;
    }
}

// 从内存池分配
void* mempool_alloc(MemoryPool* pool, u32 size) {
    if (!pool || size == 0) return NULL;
    
    // 简化实现：使用池中的简单分配器
    if (pool->used + size > pool->size) {
        return NULL;
    }
    
    void* ptr = (void*)(pool->base_address + pool->used);
    pool->used += size;
    pool->allocations++;
    
    if (pool->used > pool->peak_usage) {
        pool->peak_usage = pool->used;
    }
    
    return ptr;
}

// 释放内存池中的内存
ErrorCode mempool_free(MemoryPool* pool, void* ptr) {
    if (!pool || !ptr) return ERR_INVALID_ARG;
    
    // 简化实现：不真正释放，只更新统计
    pool->frees++;
    return ERR_SUCCESS;
}

// 销毁内存池
ErrorCode mempool_destroy(MemoryPool* pool) {
    if (!pool) return ERR_INVALID_ARG;
    
    kprintf("Destroying memory pool: %s (ID: %d)\n", pool->name, pool->id);
    
    // 从池数组移除
    for (u32 i = 0; i < MAX_MEMPOOLS; i++) {
        if (memory_pools[i] == pool) {
            memory_pools[i] = NULL;
            break;
        }
    }
    
    // 释放池内存
    memory_free((void*)pool->base_address);
    
    // 释放池结构
    memory_free(pool);
    memory_pool_count--;
    
    return ERR_SUCCESS;
}

// 查找内存池
MemoryPool* mempool_find(const char* name) {
    for (u32 i = 0; i < MAX_MEMPOOLS; i++) {
        if (memory_pools[i] && strcmp(memory_pools[i]->name, name) == 0) {
            return memory_pools[i];
        }
    }
    return NULL;
}

MemoryPool* mempool_find_by_id(u32 id) {
    for (u32 i = 0; i < MAX_MEMPOOLS; i++) {
        if (memory_pools[i] && memory_pools[i]->id == id) {
            return memory_pools[i];
        }
    }
    return NULL;
}

// 获取内存统计
void memory_get_stats(MemoryStats* stats) {
    if (stats) {
        stats->total = memory_total;
        stats->used = memory_used;
        stats->free = memory_free;
        stats->blocks = memory_block_count;
        stats->pools = memory_pool_count;
    }
}

// 获取可用内存
u32 memory_get_free(void) {
    return memory_free;
}

// 列出所有内存池
void mempool_list_all(void) {
    kprintf("\n=== Memory Pools (%d) ===\n", memory_pool_count);
    kprintf("ID   Name                Type     Size     Used\n");
    
    for (u32 i = 0; i < MAX_MEMPOOLS; i++) {
        MemoryPool* pool = memory_pools[i];
        if (pool) {
            kprintf("%-4d %-20s %-8s %-8d %-8d\n", 
                    pool->id,
                    pool->name,
                    mempool_type_to_string(pool->type),
                    pool->size,
                    pool->used);
        }
    }
}

// 内存池类型转换为字符串
const char* mempool_type_to_string(MemoryPoolType type) {
    switch (type) {
        case MEM_POOL_DEFAULT: return "DEFAULT";
        case MEM_POOL_SMALL:   return "SMALL";
        case MEM_POOL_MEDIUM:  return "MEDIUM";
        case MEM_POOL_LARGE:   return "LARGE";
        case MEM_POOL_SPECIAL: return "SPECIAL";
        default:               return "UNKNOWN";
    }
}