#pragma once

/* Kernel Management API.
 * A kernel is a main driver for computing fractal
 * Usually kernel represents an algorithm itself with
 * its own properties which can be managed with the interface
 * described bellow.
 * Various approaches can contain various algorithms and data structures inside
 * but all of them must fit into some common properties which all kernels
 * should have to interact with the program.
 *
 * Kernels are invoked by the scheduler in parallel, so they must be thread-safe.
 * For performance reasons kernel process functions should not contain any blocking code, dynamic memory allocations
 * and IO operations.
 *
 * Examples of kernels can be found in kernel_modules/kernels
 */

#include <stdint.h>
#include <mdb/surface/surface.h>

typedef struct _mdb_kernel mdb_kernel;

/* Create mandelbrot kernel of given type
 * kernel_name must point to the name of a kernel to load
 * A new kernel creates with default preset parameters.
 */
int mdb_kernel_create(mdb_kernel** pmdb, const char* kernel_name);

/* Destroy kernel and release all resources */
void mdb_kernel_destroy(mdb_kernel* mdb);

/* Create a new event in the kernel */
int mdb_kernel_event(mdb_kernel* mdb, int event_type, void* event);

/* Set dimensions of the kernel */
int mdb_kernel_set_size(mdb_kernel* mdb, uint32_t width, uint32_t height);

/* Set surface to the kernel
 */
int mdb_kernel_set_surface(mdb_kernel* mdb, surface* surf);

/* Computes values for mandelbrot set surface of given surface area.
 * This invokes the main algorithm of the kernel and writes computed values
 * into surface addressing it like: surface[y * width + x] = value.
 * This function invokes by the scheduler in parallel with small input blocks ( see scheduler )
 * The called kernel function must be thread-safe, should not contain any blocking code, dynamic memory allocations
 * and IO operations.
 */
void mdb_kernel_process_block(mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);
