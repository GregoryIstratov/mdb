#pragma once


#if defined(__cplusplus)
extern "C" {
#endif

void mdbt_kernel(unsigned long x0, unsigned long x1, unsigned long y0, unsigned long y1);

void mdbt_set_size(unsigned long width, unsigned long height);

void mdbt_set_scale(float scale);

void mdbt_set_shift(float shift_x, float shift_y);

void mdbt_set_bailout(unsigned int bailout);

void mdbt_set_surface(float* buffer);

void mdbt_compute_transpose_offset(void);

unsigned long sample_rdtsc(void);

unsigned long sample_rdtscp(void);

#if defined(__cplusplus)
}
#endif