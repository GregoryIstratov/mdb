#pragma once

#include <stdint.h>

#include <mdb/config/config.h>

typedef float mdb_float_t;

#define MDB_FLOAT_C(x) ((mdb_float_t)x)

struct _mdb_kernel;

typedef void(* mdb_block_fun)(struct _mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);

struct _mdb_kernel
{
    int bailout;
    uint32_t width;
    uint32_t height;
    mdb_float_t shift_x;
    mdb_float_t shift_y;
    mdb_float_t scale;
    mdb_float_t width_r;
    mdb_float_t height_r;
    mdb_float_t aspect_ratio;
    mdb_block_fun block_fun;
    float* f32surface;
    int kernel_type;
};


#if defined(MDB_ENABLE_AVX2_FMA_KERNEL)

void mdb_kernel_process_block_avx2_fma(struct _mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);

#endif

#if defined(MDB_ENABLE_AVX2_KERNEL)

void mdb_kernel_process_block_avx2(struct _mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);

#endif

#if defined(MDB_ENABLE_NATIVE_KERNEL)

void mdb_kernel_process_block_native(struct _mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);

#endif

void mdb_kernel_process_block_generic(struct _mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);