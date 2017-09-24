#pragma once
#include <stdint.h>

typedef void (*mdb_kernel_ext_process_block_t)(uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);

typedef void (*mdb_kernel_ext_set_size_t)(uint32_t width, uint32_t height);

typedef void (*mdb_kernel_ext_set_scale_t)(float scale);

typedef void (*mdb_kernel_ext_set_shift_t)(float shift_x, float shift_y);

typedef void (*mdb_kernel_ext_set_bailout_t)(uint32_t bailout);

typedef void (*mdb_kernel_ext_set_surface_t)(float* buffer);

typedef void (*mdb_kernel_ext_submit_changes_t)(void);