#pragma once
#include <stdint.h>
#include <mdb/tools/utils.h>
#include <mdb/tools/log.h>

__always_inline static void surface_set_pixels(float* restrict surface, uint32_t width, uint32_t height,
                                               const float* restrict pixels, uint32_t n, uint32_t x, uint32_t y)
{
#if !defined(NDEBUG) && defined(MDB_KERNEL_DEBUG)
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
                LOG_WARN("Discarding pixel (%i,%i) due to bound (%i,%i) overflow",
                            xi, y, width, height);
                break;
            }
        }

    }
    else
    {
            LOG_WARN("Discarding all pixels at y = %i due to bound (%i,%i) overflow",
                        y, width, height);
    }
#else
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
#endif
}