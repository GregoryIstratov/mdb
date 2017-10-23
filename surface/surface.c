#include "surface.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <tools/log.h>
#include <tools/compiler.h>
#include <tools/image_hdr.h>
#include <tools/error_codes.h>
#include <tools/mem.h>


static
int surface_create_buffer(void** pbuffer, uint32_t width, uint32_t height,
                          int type)
{
        size_t size     = width * height;
        size_t mem_size = size * sizeof(float);
        size_t align    = 4096; /* force to allocate it on a new mem page */

        float* surface;

        /* At this moment the only supported type is float32
         * so we omit type param */
        UNUSED_PARAM(type);

        surface = malloc_aligned(mem_size, align);
        if(surface == NULL)
        {
                LOG_ERROR("Failed to allocate memory");
                return MDB_FAIL;
        }

        memset(surface, 0, mem_size);

        *pbuffer = (void*)surface;

        return MDB_SUCCESS;
}


int surface_create(struct surface** psurf, uint32_t width, uint32_t height,
                   int flags)
{
        struct surface* surf;

        *psurf = calloc(1, sizeof(**psurf));
        surf = *psurf;

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

                return MDB_SUCCESS;
        }

        if(flags & SURFACE_BUFFER_EXT)
        {
                return MDB_SUCCESS;
        }


        LOG_ERROR("Flags are not specified right");
        return MDB_FAIL;
}

void surface_destroy(struct surface* surf)
{
        if(!surf)
                return;

        if(surf->data_need_free)
        {
                free_aligned(surf->data);
        }

        free(surf);
}

void surface_set_buffer(struct surface* surf, void* buffer)
{
        surf->data = (float*)buffer;
}



int surface_save_image_hdr(struct surface* surf, const char* filename)
{
        errno = 0;
        if (image_hdr_save_r32(filename, surf->width, surf->height, surf->data))
        {
                return MDB_FAIL;
        }

        return MDB_SUCCESS;

}