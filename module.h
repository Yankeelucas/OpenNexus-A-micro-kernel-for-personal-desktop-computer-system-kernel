// SPDX-License-Identifier: GPL-3.0-or-later
// module.h - 模块系统头文件
#ifndef __MODULE_H__
#define __MODULE_H__

#include "kernel.h"

// 模块头结构
typedef struct {
    u32 magic;
    u32 version;
    u32 text_size;
    u32 data_size;
    u32 bss_size;
    u32 export_offset;
    u32 import_offset;
    u32 checksum;
} ModuleHeader;

#define MODULE_MAGIC 0x4D4F4455  // "MODU"

// 模块导出函数表
typedef struct {
    // 必需函数
    ErrorCode (*init)(void* params);
    ErrorCode (*exit)(void);
    ErrorCode (*query)(ModuleInfo* info);
    
    // 可选函数指针（最多15个）
    void* functions[15];
    
    // 模块信息
    ModuleInfo info;
    
    // 私有数据
    void* private_data;
} ModuleExportTable;

// 模块管理函数
ErrorCode module_system_init(void);
ErrorCode module_load(const char* filename, ModuleInfo* info);
ErrorCode module_unload(const char* name);
ErrorCode module_query(const char* name, ModuleInfo* info);
void* module_call(const char* module_name, u32 function_index, void* params);
void module_list_all(void);

// 模块查找
ModuleEntry* module_find(const char* name);

#endif // __MODULE_H__