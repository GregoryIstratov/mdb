#pragma once

#include <stdint.h>

#include <mdb/config/config.h>
#include <mdb/kernel/mdb_kernel_ext.h>

typedef float mdb_float_t;

#define MDB_FLOAT_C(x) ((mdb_float_t)x)

struct _mdb_kernel;

typedef void(* mdb_block_fun)(struct _mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);

struct _mdb_kernel
{
    /* Common properties */
    int bailout;
    uint32_t width;
    uint32_t height;
    mdb_float_t shift_x;
    mdb_float_t shift_y;
    mdb_float_t scale;
    float* f32surface;

    /* Internal kernels properties */
    mdb_float_t width_r;
    mdb_float_t height_r;
    mdb_float_t aspect_ratio;

    /* Internal kernels block function.
     * Internal kernels have full access
     * to this structure, so we need
     * only one function for communication
     * */
    mdb_block_fun block_fun;

    /* External kernels API.
     * External kernels can have various
     * internal structures and algorithms inside
     * So we need an API to inform them about changes
     * in parameters coming from outside.
     * */

    mdb_kernel_ext_process_block_t  ext_block_fun;
    mdb_kernel_ext_set_bailout_t    ext_set_bailout_fun;
    mdb_kernel_ext_set_size_t       ext_set_size_fun;
    mdb_kernel_ext_set_scale_t      ext_set_scale_fun;
    mdb_kernel_ext_set_shift_t      ext_set_shift_fun;
    mdb_kernel_ext_set_surface_t    ext_set_surface_fun;
    mdb_kernel_ext_submit_changes_t ext_submit_changes_fun;


    /* Misc properties */
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