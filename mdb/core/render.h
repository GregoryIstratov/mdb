#pragma once

#include <stdint.h>
#include <mdb/kernel/kernel_cpu.h>
#include <mdb/sched/rsched.h>

void render_run(rsched* sched, mdb_kernel* kernel, uint32_t width, uint32_t height, uint32_t grain);