// SPDX-License-Identifier: GPL-3.0-or-later
// template.c - 模块开发模板
#include "kernel.h"

// 模块信息
static ModuleInfo module_info = {
    .name = "TemplateModule",
    .version = "1.0.0",
    .author = "System Developer",
    .description = "Template module for demonstration",
    .type = MODULE_TYPE_UTILITY,
    .api_version = 1,
    .flags = 0
};

// 模块私有数据
typedef struct {
    u32 initialization_count;
    u32 function_calls;
    // 添加更多私有数据...
} ModulePrivateData;

static ModulePrivateData private_data;

// 模块初始化函数
static ErrorCode module_init(void* params) {
    kprintf("TemplateModule: Initializing...\n");
    
    // 初始化私有数据
    memset(&private_data, 0, sizeof(private_data));
    private_data.initialization_count = 1;
    
    kprintf("TemplateModule: Ready\n");
    return ERR_SUCCESS;
}

// 模块退出函数
static ErrorCode module_exit(void) {
    kprintf("TemplateModule: Exiting...\n");
    kprintf("  Function calls: %d\n", private_data.function_calls);
    
    // 清理资源...
    
    return ERR_SUCCESS;
}

// 模块查询函数
static ErrorCode module_query(ModuleInfo* info) {
    if (info) {
        *info = module_info;
    }
    return ERR_SUCCESS;
}

// ======================
// 模块功能函数 (示例)
// ======================

// 函数1: 返回模块状态
static void* function1(void* params) {
    private_data.function_calls++;
    
    static char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "TemplateModule status:\n"
             "  Initializations: %d\n"
             "  Function calls: %d",
             private_data.initialization_count,
             private_data.function_calls);
    
    return buffer;
}

// 函数2: 执行计算
static void* function2(void* params) {
    private_data.function_calls++;
    
    if (!params) return NULL;
    
    u32* numbers = (u32*)params;
    u32 count = numbers[0];
    u32 sum = 0;
    
    for (u32 i = 1; i <= count; i++) {
        sum += numbers[i];
    }
    
    numbers[0] = sum;  // 用结果替换第一个元素
    return params;
}

// 函数3: 内存测试
static void* function3(void* params) {
    private_data.function_calls++;
    
    u32* result = (u32*)memory_alloc(sizeof(u32));
    if (result) {
        *result = 0xDEADBEEF;  // 测试值
    }
    
    return result;
}

// 更多函数...
// function4, function5, ..., function15

// ======================
// 模块导出表
// ======================
static ModuleExportTable exports = {
    .init = module_init,
    .exit = module_exit,
    .query = module_query,
    
    // 注册功能函数
    .functions = {
        function1,
        function2,
        function3,
        // 注册更多函数...
        NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL,
        NULL, NULL
    },
    
    .info = module_info,
    .private_data = &private_data
};

// 模块入口点（由模块加载器调用）
ModuleExportTable* module_entry(void) {
    return &exports;
}