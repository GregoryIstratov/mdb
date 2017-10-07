#pragma once

#include <stdint.h>

#include <mdb/config/config.h>

#include <mdb/kernel/bits/mdb_kernel_meta.h>

#include <mdb/surface/surface.h>

struct _mdb_kernel;

typedef int (*mdb_kernel_init_t)(void);

typedef int (*mdb_kernel_shutdown_t)(void);

/* Returns bitmask of required cpu features by the kernel
 * 0 if no special requirements
 * */
typedef int (*mdb_kernel_cpu_features_t)(void);

typedef int (*mdb_kernel_metadata_query_t)(int query, char* buff, uint32_t buff_size);

typedef int (*mdb_kernel_event_handler_t)(int type, void* event_ctx);

typedef void (*mdb_kernel_process_block_t)(uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);

typedef int (*mdb_kernel_set_size_t)(uint32_t width, uint32_t height);

typedef int (*mdb_kernel_set_surface_t)(surface* surf);

enum
{
    /* Kernel states */
    MDB_KRN_NOINIT  = 0,
    MDB_KRN_LOADED  = 1,
    MDB_KRN_INITED  = 2
};

struct _mdb_kernel
{
    /* Kernel API */

    mdb_kernel_init_t           init_fun;
    mdb_kernel_shutdown_t       shutdown_fun;
    mdb_kernel_cpu_features_t   cpu_features_fun;
    mdb_kernel_metadata_query_t metadata_query_fun;
    mdb_kernel_event_handler_t  event_handler_fun;
    mdb_kernel_process_block_t  block_fun;
    mdb_kernel_set_size_t       set_size_fun;
    mdb_kernel_set_surface_t    set_surface_fun;

    /* Dynamic kernel handle */
    void* dl_handle;

    int state;
};