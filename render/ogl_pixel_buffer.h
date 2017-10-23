#pragma once
#include <glad/glad.h>

typedef void (*ogl_data_update_callback)(void*, void*);

typedef struct _ogl_pixel_buffer ogl_pixel_buffer;

void ogl_pbo_create(ogl_pixel_buffer** ppbo, uint32_t width, uint32_t height,
                    ogl_data_update_callback callback, void* ctx);

void ogl_pbo_update(ogl_pixel_buffer* pbo, GLsync* sync);

void ogl_pbo_bind_texture0(ogl_pixel_buffer* pbo);

void ogl_pbo_destroy(ogl_pixel_buffer* pbo);

void ogl_pbo_resize(ogl_pixel_buffer* pbo, GLsync* sync,
                    uint32_t width, uint32_t height);

