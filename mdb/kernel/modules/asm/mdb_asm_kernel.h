#pragma once

#if 0
#include <stdint.h>

void mdb_asm_kernel_metadata_query(int flags, char* buff, uint32_t buff_size);

void mdb_asm_kernel_init(void);

void mdb_asm_kernel_shutdown(void);

void mdb_asm_kernel_process_block(uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);

void mdb_asm_kernel_set_size(uint32_t width, uint32_t height);

void mdb_asm_kernel_set_scale(float scale);

void mdb_asm_kernel_set_shift(float shift_x, float shift_y);

void mdb_asm_kernel_set_bailout(uint32_t bailout);

void mdb_asm_kernel_set_surface(float* buffer);

void mdb_asm_kernel_submit_changes(void);

unsigned long sample_rdtsc(void);

unsigned long sample_rdtscp(void);
#endif