#include "benchmark.h"

#include <malloc.h>
#include <mdb/tools/compiler.h>
#include <mdb/tools/timer.h>
#include <mdb/kernel/mdb_kernel.h>
#include <mdb/sched/rsched.h>

#include <mdb/tools/atomic.h>
#include <limits.h>
#include <mdb/tools/log.h>


static
void benchmark_proc_fun(uint32_t x0, uint32_t x1, uint32_t y0,
                               uint32_t y1, void* ctx)
{
        struct perf_timer tm_block;
        struct benchmark* bench = ctx;

        uint64_t elapsed_time, min_time, max_time;

        perf_timer_start(&tm_block);

        mdb_kernel_process_block(bench->kernel, x0, x1, y0, y1);

        perf_timer_stop(&tm_block);


        elapsed_time = perf_timer_diff_ns(&tm_block);

        min_time = atomic_load(&bench->min_block_time);
        while(min_time > elapsed_time)
        {
                if(atomic_compare_exchange(&bench->min_block_time,
                                           &min_time, elapsed_time))
                        break;
        }

        max_time = atomic_load(&bench->max_block_time);
        while(max_time < elapsed_time)
        {
                if(atomic_compare_exchange(&bench->max_block_time,
                                           &max_time, elapsed_time))
                        break;
        }

        atomic_fetch_add_relaxed(&bench->total_block_time, elapsed_time);
        atomic_fetch_add_relaxed(&bench->block_count, 1);
}

static
void benchmark_proc_dummy_fun(uint32_t x0, uint32_t x1, uint32_t y0,
                                     uint32_t y1, void* ctx)
{
        UNUSED_PARAM(x0);
        UNUSED_PARAM(x1);
        UNUSED_PARAM(y0);
        UNUSED_PARAM(y1);
        UNUSED_PARAM(ctx);
}

void benchmark_create(struct benchmark** pbench, uint32_t runs,
                      struct mdb_kernel* kernel,
                      struct rsched* sched)
{
        struct benchmark* bench;

        *pbench = calloc(1, sizeof(**pbench));
        bench = *pbench;

        bench->kernel = kernel;
        bench->sched = sched;

        bench->runs = runs;

        atomic_store(&bench->total_block_time, 0);
        atomic_store(&bench->min_block_time, UINT64_MAX);
        atomic_store(&bench->max_block_time, 0);
        atomic_store(&bench->block_count, 0);

        rsched_set_user_context(bench->sched, &benchmark_proc_fun, bench);
}

void benchmark_destroy(struct benchmark* bench)
{
        free(bench);
}

static
void benchmark_run_kernel(struct benchmark* bench)
{
        struct perf_timer tm_kernel;
        uint32_t runs = bench->runs;
        uint32_t run = 0;

        perf_timer_start(&tm_kernel);

        while(run < runs)
        {
                rsched_host_yield(bench->sched);
                rsched_requeue(bench->sched);

                ++run;
        }

        perf_timer_stop(&tm_kernel);
        bench->total_exec_time = perf_timer_diff_sec(&tm_kernel);
}

void benchmark_run(struct benchmark* bench)
{
        benchmark_run_kernel(bench);
}

void benchmark_print_summary(struct benchmark* bench)
{
        uint32_t threads = rsched_threads_count(bench->sched);

        double block_ms = ns_to_ms(atomic_load(&bench->total_block_time) / threads);
        double min_block_ms = ns_to_ms(atomic_load(&bench->min_block_time));
        double max_block_ms = ns_to_ms(atomic_load(&bench->max_block_time));

        uint64_t block_count = atomic_load(&bench->block_count);

        PARAM_INFO("Total blocks", "%lu", block_count);
        PARAM_INFO("Avg block time", "%f ms", (block_ms / block_count));
        PARAM_INFO("Min block time", "%f ms", min_block_ms);
        PARAM_INFO("Max block time", "%f ms", max_block_ms);
        PARAM_INFO("Total block time", "%f sec", (block_ms / 1000.0));
        PARAM_INFO("Total execution time", "%f sec", bench->total_exec_time);
        PARAM_INFO("Total runs", "%i", bench->runs);
        PARAM_INFO("Avg FPS", "%f",
                   ((double)bench->runs / bench->total_exec_time));
}
