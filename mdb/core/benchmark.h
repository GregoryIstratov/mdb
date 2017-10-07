#pragma once

#include <mdb/sched/rsched.h>
#include <mdb/kernel/mdb_kernel.h>

typedef struct _benchmark benchmark;

void benchmark_create(benchmark** pbench, uint32_t runs, mdb_kernel* kernel, rsched* sched);
void benchmark_destroy(benchmark* bench);
void benchmark_run(benchmark* bench);
void benchmark_print_summary(benchmark* bench);