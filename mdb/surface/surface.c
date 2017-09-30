#include "surface.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <mdb/tools/log.h>
#include <mdb/tools/compiler.h>
#include <mdb/tools/image/image_hdr.h>

struct _surface
{
    uint32_t width;
    uint32_t height;

    float* data;
    int data_need_free;
};

static int surface_create_buffer(void** pbuffer, uint32_t width, uint32_t height, int type)
{
    /* At this moment only supported type is float32
     * so we omit type param */
    UNUSED_PARAM(type);

    float* surface = NULL;
    size_t size = width * height;
    size_t mem_size = size * sizeof(float);
    size_t align = 4096; //force to allocate it on a new page

    int res = posix_memalign((void**) &surface, align, mem_size);
    if (res)
    {
        if (ENOMEM == res)
            LOG_ERROR("There was insufficient memory available to satisfy the request.");
        if (EINVAL == res)
            LOG_ERROR("Alignment is not a power of two multiple of sizeof (void *).");

        return -1;
    }

    memset(surface, 0, mem_size);

    *pbuffer = (void*)surface;

    return 0;
}


int surface_create(surface** psurf, uint32_t width, uint32_t height, int flags)
{
    *psurf = calloc(1, sizeof(surface));
    surface* surf = *psurf;

    surf->width  = width;
    surf->height = height;

    if(flags & SURFACE_BUFFER_CREATE)
    {
        if(surface_create_buffer((void**)&surf->data, width, height, 0))
        {
            LOG_ERROR("Failed to create surface buffer.");
            return -1;
        }

        surf->data_need_free = 1;

        return 0;
    }

    if(flags & SURFACE_BUFFER_EXT)
    {
        return 0;
    }


    LOG_ERROR("Flags are not specified right");
    return -1;
}

void surface_destroy(surface* surf)
{
    if(!surf)
        return;

    if(surf->data_need_free)
    {
        free(surf->data);
    }

    free(surf);
}

void surface_set_buffer(surface* surf, void* buffer)
{
    surf->data = (float*)buffer;
}

void surface_set_pixels(surface* surf, uint32_t x, uint32_t y, uint32_t n, void* pix_data)
{
    /* This is a temporary implementation only with supporting float 32 buffers
     */

    uint32_t height = surf->height;
    uint32_t width  = surf->width;
    float* pix_buffer = surf->data;

    if(likely(y < height))
    {
        size_t idx_y = y * width;
        for (uint32_t k = 0; k < n; ++k)
        {
            uint32_t xi = x + k;
            size_t idx = idx_y + xi;
            if (likely(xi < width))
            {
                pix_buffer[idx] = ((float*)pix_data)[k];
            }
            else
            {
                break;
            }
        }

    }
}

int surface_save_image_hdr(surface* surf, const char* filename)
{
    errno = 0;
    if (image_hdr_save_r32(filename, surf->width, surf->height, surf->data))
    {
        return -1;
    }

    return 0;

}