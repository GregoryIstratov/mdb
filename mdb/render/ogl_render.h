#pragma once

#include <stdint.h>

typedef void (*data_update_callback)(void*, void*);

typedef void(*ogl_render_key_callback)(int, void*);

typedef void(*ogl_render_resize_callback)(int,int, void*);

typedef struct _ogl_render ogl_render;

void ogl_render_create(ogl_render** rend, uint32_t width, uint32_t height, data_update_callback cb, void* user_ctx, ogl_render_key_callback key_cb);

void ogl_render_set_resize_callback(ogl_render* rend, ogl_render_resize_callback cb);

void ogl_render_render_loop(ogl_render* rend);

void ogl_render_destroy(ogl_render* rend);

void ogl_render_colors_enabled(int status);