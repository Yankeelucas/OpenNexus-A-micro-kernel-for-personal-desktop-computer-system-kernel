// SPDX-License-Identifier: GPL-3.0-or-later
#include <gui/graphics.h>
#include <kernel.h>

// 空的图形操作表
gfx_operations_t gfx_ops = {
    .init = NULL,
    .deinit = NULL,
    .get_context = NULL,
    .set_mode = NULL,
    .put_pixel = NULL,
    .fill_rect = NULL,
    .draw_line = NULL,
    .clear = NULL,
    .swap_buffers = NULL,
    .set_double_buffering = NULL,
    .draw_char = NULL,
    .draw_string = NULL
};

// 图形初始化（桩函数）
err_t gfx_init(u32 width, u32 height, u32 bpp) {
    (void)width; (void)height; (void)bpp;
    kprintf("Graphics: GUI module not loaded\n");
    return ERR_NOT_IMPLEMENTED;
}

// 图形清理（桩函数）
err_t gfx_deinit(void) {
    return ERR_SUCCESS;  // 总是成功，因为什么都没做
}

// 获取图形上下文（桩函数）
struct gfx_context* gfx_get_context(void) {
    kprintf("Graphics: No graphics context available\n");
    return NULL;
}

// 绘制像素（桩函数）
err_t gfx_put_pixel(u32 x, u32 y, u32 color) {
    (void)x; (void)y; (void)color;
    return ERR_NOT_IMPLEMENTED;
}

// 填充矩形（桩函数）
err_t gfx_fill_rect(u32 x, u32 y, u32 w, u32 h, u32 color) {
    (void)x; (void)y; (void)w; (void)h; (void)color;
    return ERR_NOT_IMPLEMENTED;
}

// 清屏（桩函数）
err_t gfx_clear(u32 color) {
    (void)color;
    return ERR_NOT_IMPLEMENTED;
}