#pragma once

#include <stdint.h>

#include <mdb/config/config.h>

#include <mdb/kernel/bits/mdb_kernel_meta.h>

#include <mdb/surface/surface.h>

#define MDB_FLOAT_C(x) ((float)(x))

struct _mdb_kernel;

typedef void (*mdb_kernel_init_t)(void);

typedef void (*mdb_kernel_shutdown_t)(void);

/* Returns bitmask of required cpu features by the kernel
 * 0 if no special requirements
 * */
typedef int (*mdb_kernel_cpu_features_t)(void);

typedef int (*mdb_kernel_metadata_query_t)(int query, char* buff, uint32_t buff_size);

typedef void (*mdb_kernel_process_block_t)(uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);

typedef void (*mdb_kernel_set_size_t)(uint32_t width, uint32_t height);

typedef void (*mdb_kernel_set_scale_t)(float scale);

typedef void (*mdb_kernel_set_shift_t)(float shift_x, float shift_y);

typedef void (*mdb_kernel_set_bailout_t)(uint32_t bailout);

typedef void (*mdb_kernel_set_surface_t)(surface* surf);

typedef void (*mdb_kernel_submit_changes_t)(void);

struct _mdb_kernel
{
    /* External kernels API.
     * External kernels can have various
     * internal structures and algorithms inside
     * So we need an API to inform them about changes
     * in parameters coming from outside.
     * */

    mdb_kernel_init_t           init_fun;
    mdb_kernel_shutdown_t       shutdown_fun;
    mdb_kernel_cpu_features_t   cpu_features_fun;
    mdb_kernel_metadata_query_t metadata_query_fun;
    mdb_kernel_process_block_t  block_fun;
    mdb_kernel_set_bailout_t    set_bailout_fun;
    mdb_kernel_set_size_t       set_size_fun;
    mdb_kernel_set_scale_t      set_scale_fun;
    mdb_kernel_set_shift_t      set_shift_fun;
    mdb_kernel_set_surface_t    set_surface_fun;
    mdb_kernel_submit_changes_t submit_changes_fun;

    /* Dynamic kernel handle */
    void* dl_handle;
};