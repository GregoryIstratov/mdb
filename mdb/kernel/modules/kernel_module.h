#pragma once

#include <string.h>

#include <mdb/kernel/bits/mdb_kernel_meta.h>
#include <mdb/tools/bits/cpu_features.h>
#include <mdb/tools/utils.h>
#include <stdint.h>

__always_inline static void surface_set_pixels(float* restrict surface, uint32_t width, uint32_t height,
                                               const float* restrict pixels, uint32_t n, uint32_t x, uint32_t y)
{
    if(likely(y < height))
    {
        size_t idx_y = y * width;
        for (uint32_t k = 0; k < n; ++k)
        {
            uint32_t xi = x + k;
            size_t idx = idx_y + xi;
            if (likely(xi < width))
            {
                surface[idx] = pixels[k];
            }
            else
            {
                break;
            }
        }

    }
}

__always_inline static int metadata_copy(const char* meta, char* dst, uint32_t dst_size)
{
    size_t meta_sz = strlen(meta) + 1;
    if(meta_sz > dst_size)
        return MDB_QUERY_BUFFER_OVERFLOW;

    memcpy(dst, meta, meta_sz);

    return MDB_QUERY_OK;
}