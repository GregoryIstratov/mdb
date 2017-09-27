#pragma once

/* Mandelbrot kernel management.
 * A kernel is a main driver for computing mandelbrot set
 * Usually kernel represents an algorithm itself with
 * its own properties which can be managed with the interface
 * described bellow.
 * Various approaches can contain various algorithms and data structures inside
 * but all of them must fit into some common properties which all kernels
 * should have to interact with the program.
 *
 * A brief list of these properties:
 * - Resolution Width x Height
 * - Shift coordinates of a central point
 * - Scale - scale of the current coordinates
 * - Bailout - a maximum number of iterations when the point considered bailed out.
 * - Surface - 32-bit float texture WIDTH x HEIGHT size
 * - Process function that takes current surface coordinates, computes values for each pixel
 * of the given block of the surface area, and writes these values to the area data *
 *
 * Kernels can be internal and external
 * Internal kernels are kernels which are built inside the program
 * External kernels are kernels which built as a module and can be loaded dynamically
 * without whole program recompilation
 * External kernels must implement special API described in mdb/external/mdb_kernel_ext.h
 * and put into special folder so dynamic loader can find and load them.
 *
 * Kernels are invoked by the scheduler in parallel, so they must be thread-safe.
 * For performance reasons kernels should not contain any blocking code, dynamic memory allocations
 * and IO operations.
 *
 * Examples of kernels with good performance can be found in sources like avx2, avx2_fma, avx2_fma_asm
 * which were profiled many times and very good tuned.
 */

#include <stdint.h>

typedef struct _mdb_kernel mdb_kernel;

enum
{
    MDB_KERNEL_AVX2_FMA,
    MDB_KERNEL_AVX2,
    MDB_KERNEL_GENERIC,
    MDB_KERNEL_NATIVE,
    MDB_KERNEL_AVX2_FMA_ASM,
    MDB_KERNEL_EXTERNAL
};

/* Create mandelbrot kernel of given type
 * if kernel_type is MDB_KERNEL_EXTERNAL
 * then ext_kernel must point to the name of external kernel to load
 * otherwise NULL.
 * A new kernel creates with default preset parameters.
 */
int mdb_kernel_create(mdb_kernel** pmdb, int kernel_type, const char* ext_kernel);

/* Destroy kernel and release all resources */
void mdb_kernel_destroy(mdb_kernel* mdb);

/* Set dimensions of the kernel */
void mdb_kernel_set_size(mdb_kernel* mdb, uint32_t width, uint32_t height);

/* Set scale of complex surface */
void mdb_kernel_set_scale(mdb_kernel* mdb, float scale);

/* Set shift of the central point of complex surface */
void mdb_kernel_set_shift(mdb_kernel* mdb, float shift_x, float shift_y);

/* Set bailout */
void mdb_kernel_set_bailout(mdb_kernel* mdb, uint32_t bailout);

/* Set surface to the kernel
 * Surface must be a continuous memory block of 32 bit floats of size WIDTH x HEIGHT
 */
void mdb_kernel_set_surface(mdb_kernel* mdb, float* f32surface);

/* Some kernels keep their internal parameters precomputed
 * these parameters can have circular dependencies and/or
 * can be heavy to compute, etc.
 * Such kernels can implement relaxed parameter setup, this means
 * for example if scale changed a kernel might not immediately
 * update its internal parameters and parameters will be updated
 * only after this function call.
 * This allows a batch parameters changing to decrease performance impact.
 */
void mdb_kernel_submit_changes(mdb_kernel* mdb);


/* Computes values for mandelbrot set surface of given surface area.
 * This invokes the main algorithm of the kernel and writes computed values
 * into surface addressing it like: surface[y * width + x] = value.
 * This function invokes by the scheduler in parallel with small input blocks ( see scheduler )
 * The called kernel function must be thread-safe, should not contain any blocking code, dynamic memory allocations
 * and IO operations.
 */
void mdb_kernel_process_block(mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);
