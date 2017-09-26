#pragma once

/* Save linear float 32 bit data represented as single channel (r32) rectangle width x height
 * to Radiance HDR RGBE image format, with replicating color to all channels.
 * https://en.wikipedia.org/wiki/RGBE_image_format
 * Returns 0 on success.
 * */
int image_hdr_save_r32(const char* filename, int width, int height, float* f32data);