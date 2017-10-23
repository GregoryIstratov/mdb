#pragma once

#include <stdint.h>
#include <tools/compiler.h>

enum
{
        /* Creates a buffer internally
         * A buffer type must be specified */
        SURFACE_BUFFER_CREATE   = 1,

        /* This flag informs that a surface buffer will be set
         * from outside and no need to create a buffer at initialize. */
        SURFACE_BUFFER_EXT      = 1<<1,


        /* A surface buffer data type */
        SURFACE_BUFFER_F32      = 1<<20
};

struct surface
{
        float* data;

        uint32_t width;
        uint32_t height;

        int data_need_free;
};

/* Create a surface with the given parameters */
int surface_create(struct surface** psurf, uint32_t width, uint32_t height,
                   int flags);

/* Destroy the surface releasing all acquired resources */
void surface_destroy(struct surface* surf);

/* Set a buffer to the surface
 * A surface must be created with a SURFACE_BUFFER_EXT flag */
void surface_set_buffer(struct surface* surf, void* buffer);

/* Set pixel values to the surface at the specified coordinates.
 *
 * Stores pixels in a row by x according number of pixels specified by n
 * addressing underlying buffer by y * width + x.
 * If x + n >= width, pixels are not fitting this will be discarded.
 * If y >= height all pixels will be discarded.
 */
static inline
void surface_set_pixels(struct surface* surf, uint32_t x, uint32_t y,
                        uint32_t n, void* pix_data)
{
        /* This is a temporary implementation only with supporting
         * float 32 buffers
         */

        uint32_t height = surf->height;
        uint32_t width  = surf->width;
        float* pix_buffer = surf->data;

        size_t idx_y, idx;
        uint32_t xi, k;

        if(unlikely(y >= height))
                return;

        idx_y = y * width;
        for (k = 0; k < n; ++k)
        {
                xi = x + k;
                idx = idx_y + xi;
                if (likely(xi < width))
                        pix_buffer[idx] = ((float*)pix_data)[k];

                else
                        break;

        }


}

/* Save the surface to Radiance HDR RGBE image format.
 * https://en.wikipedia.org/wiki/RGBE_image_format
 * Returns 0 on success.
 * */
int surface_save_image_hdr(struct surface* surf, const char* filename);
