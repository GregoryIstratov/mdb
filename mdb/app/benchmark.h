#pragma once

#include <mdb/sched/rsched.h>
#include <mdb/kernel/mdb_kernel.h>

struct benchmark
{
        struct mdb_kernel* kernel;
        struct rsched* sched;

        int width, height;

        uint32_t runs;
        double total_exec_time;

        __cache_aligned
        __atomic
        uint64_t total_block_time;

        __atomic
        uint64_t min_block_time;

        __atomic
        uint64_t max_block_time;

        __atomic
        uint64_t block_count;
};

void benchmark_create(struct benchmark** pbench, uint32_t runs,
                      struct  mdb_kernel* kernel,
                      struct rsched* sched);
void benchmark_destroy(struct benchmark* bench);
void benchmark_run(struct benchmark* bench);
void benchmark_print_summary(struct benchmark* bench);
