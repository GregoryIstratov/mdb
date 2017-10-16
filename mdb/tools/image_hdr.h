#pragma once

#include <stdint.h>

/* Save linear float 32 bit data represented as a single channeled (r32)
 * rectangle of width x height size to Radiance HDR RGBE image format
 * replicating color across other channels.
 *
 * https://en.wikipedia.org/wiki/RGBE_image_format
 *
 * Returns 0 on success.
 * */
int image_hdr_save_r32(const char* filename, uint32_t width, uint32_t height,
                       float* f32data);
