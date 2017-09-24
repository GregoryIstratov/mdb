#pragma once

//#include <stdint.h>

void mdb_asm_kernel_process_block(unsigned long x0, unsigned long x1, unsigned long y0, unsigned long y1);

void mdb_asm_kernel_set_size(unsigned long width, unsigned long height);

void mdb_asm_kernel_set_scale(float scale);

void mdb_asm_kernel_set_shift(float shift_x, float shift_y);

void mdb_asm_kernel_set_bailout(unsigned int bailout);

void mdb_asm_kernel_set_surface(float* buffer);

void mdb_asm_kernel_submit_changes(void);

unsigned long sample_rdtsc(void);

unsigned long sample_rdtscp(void);