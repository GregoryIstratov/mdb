#pragma once

#include <stdint.h>
#include <mdb/tools/compiler.h>

enum
{
    /* Creates buffer internally
     * Buffer type must be specified */
    SURFACE_BUFFER_CREATE   = 1,

    /* This flag informs that surface buffer will be set
     * from outside and no need to create buffer at initialize. */
    SURFACE_BUFFER_EXT      = 1<<1,


    /* Surface buffer data type */
    SURFACE_BUFFER_F32      = 1<<20,
};

typedef struct _surface surface;

/* Create a surface with given parameters */
int surface_create(surface** psurf, uint32_t width, uint32_t height, int flags);

/* Destroy the surface releasing all acquired resources */
void surface_destroy(surface* surf);

/* Set a buffer to the surface
 * Surface must be created with SURFACE_BUFFER_EXT flag */
void surface_set_buffer(surface* surf, void* buffer);

/* Set pixel values to the surface at specified coordinates
 * Pixels stores in a row by x according count of pixels specified by n
 * addressing underlying buffer by y * width + x.
 * If x + n >= width not fitting pixels will be discarded.
 * If y >= height all pixels will be discarded.
 */
__export_sym
void surface_set_pixels(surface* surf, uint32_t x, uint32_t y, uint32_t n, void* pix_data);

/* Save surface to Radiance HDR RGBE image format.
 * https://en.wikipedia.org/wiki/RGBE_image_format
 * Returns 0 on success.
 * */
int surface_save_image_hdr(surface* surf, const char* filename);