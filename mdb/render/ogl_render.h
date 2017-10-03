#pragma once

/* OpenGL render engine
 * This engine creates memory buffer for cpu rendering
 * and then renders the buffer to the screen
 * applying shader coloring if enabled.
 *
 * This engine requires the next OpenGL features:
 * OpenGL minimum 3.2
 * GL_ARB_buffer_storage
 * GL_ARB_explicit_attrib_location
 * GL_ARB_explicit_uniform_location
 * GL_ARB_shading_language_420pack
 *
 * It's been successfully testes on:
 * Nvidia GTX+ 580 on linux with nvidia proprietary drivers
 * Intel Core i5 4670K with integrated graphics on linux (4.12 kernel) and Mesa 17.2.1
 */

#include <stdint.h>
#include <stdbool.h>

#include "bits/ogl_pixel_buffer.h"

typedef void(*ogl_render_key_callback)(void*, int, int, int, int);

typedef void(*ogl_render_resize_callback)(int,int, void*);

typedef struct _ogl_render ogl_render;

void ogl_render_create(ogl_render** rend, const char* win_title, uint32_t width, uint32_t height, void* user_ctx);

void ogl_render_init_render_target(ogl_render* rend, ogl_data_update_callback cb);

void ogl_render_init_screen(ogl_render* rend, bool color_enabled);

void ogl_render_set_resize_callback(ogl_render* rend, ogl_render_resize_callback cb);

void ogl_render_set_key_callback(ogl_render* rend, ogl_render_key_callback cb);

void ogl_render_render_loop(ogl_render* rend);

void ogl_render_destroy(ogl_render* rend);

