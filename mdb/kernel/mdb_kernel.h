#pragma once

/* Kernel Loader Management.
 *
 * A kernel is a main driver for computing a fractal.
 *
 * Usually a kernel represents an algorithm itself with its own properties
 * which can be managed with the interface described bellow.
 *
 * Various approaches can contain various algorithms and data structures inside
 * but all of them must fit into some common properties which all kernels should
 * have to interact with the program, this is called the kernel module API.
 *
 * Kernels are invoked by the scheduler in parallel, so they must be thread-safe.
 * For performance reasons kernel process functions should not contain any
 * blocking code, dynamic memory allocations and IO operations.
 *
 * Examples of kernels can be found in kernel_modules/
 */

#include <stdint.h>
#include <mdb/surface/surface.h>
#include <mdb/config/config.h>
#include <mdb/kernel/mdb_kernel_meta.h>
#include <mdb/surface/surface.h>

/* TODO kernel load parameters support
 *
 */

typedef int (*mdb_kernel_init_t)(void);

typedef int (*mdb_kernel_shutdown_t)(void);

/* Returns a bitmask of the required cpu features by the kernel
 * 0 if no special requirements
 * */
typedef int (*mdb_kernel_cpu_features_t)(void);

typedef int (*mdb_kernel_metadata_query_t)(int query, char* buff,
                                           uint32_t buff_size);

typedef int (*mdb_kernel_event_handler_t)(int type, void* event_ctx);

typedef void (*mdb_kernel_process_block_t)(uint32_t x0, uint32_t x1,
                                           uint32_t y0, uint32_t y1);

typedef int (*mdb_kernel_set_size_t)(uint32_t width, uint32_t height);

typedef int (*mdb_kernel_set_surface_t)(struct surface* surf);

enum
{
        /* Kernel states */
        MDB_KRN_NOINIT  = 0,
        MDB_KRN_LOADED  = 1,
        MDB_KRN_INITED  = 2
};

struct mdb_kernel
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

/* Create a kernel of a given type.
 * kernel_name must point to a name of the kernel to load.
 */
int mdb_kernel_create(struct mdb_kernel** pmdb, const char* kernel_name);

/* Destroy kernel and release all resources */
void mdb_kernel_destroy(struct mdb_kernel* mdb);

/* Create a new event in the kernel */
int mdb_kernel_event(struct mdb_kernel* mdb, int event_type, void* event);

/* Set dimensions of the kernel */
int mdb_kernel_set_size(struct mdb_kernel* mdb, uint32_t width, uint32_t height);

/* Set surface to the kernel */
int mdb_kernel_set_surface(struct mdb_kernel* mdb, struct surface* surf);

/* Computes values on the surface for a specific kernel.
 * This function is invoking by the scheduler in parallel ( see scheduler ).
 */
void mdb_kernel_process_block(struct mdb_kernel* mdb, uint32_t x0, uint32_t x1,
                              uint32_t y0, uint32_t y1);
