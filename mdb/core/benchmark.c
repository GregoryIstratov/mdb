#include "benchmark.h"

#include <malloc.h>
#include <mdb/tools/compiler.h>
#include <mdb/tools/timer.h>
#include <mdb/kernel/mdb_kernel.h>
#include <mdb/sched/rsched.h>

#include <mdb/tools/atomic.h>
#include <limits.h>
#include <mdb/tools/log.h>


struct _benchmark
{
    int width, height;
    mdb_kernel* kernel;
        struct rsched* sched;
    uint32_t runs;
    __atomic uint64_t total_block_time;
    __atomic uint64_t min_block_time;
    __atomic uint64_t max_block_time;
    __atomic uint64_t block_count;
    perf_timer tm_kernel;

};

static void benchmark_proc_fun(uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1, void* ctx)
{
    static __thread perf_timer tm_block;

    benchmark* bench = (benchmark*)ctx;

    uint64_t elapsed_time, min_time, max_time;

    perf_timer_start(&tm_block);

    mdb_kernel_process_block(bench->kernel, x0, x1, y0, y1);

    perf_timer_stop(&tm_block);


    elapsed_time = perf_timer_diff_ns(&tm_block);

    min_time = atomic_load(&bench->min_block_time);
    while(min_time > elapsed_time)
    {
        if(atomic_compare_exchange(&bench->min_block_time, &min_time, elapsed_time))
            break;
    }

    max_time = atomic_load(&bench->max_block_time);
    while(max_time < elapsed_time)
    {
        if(atomic_compare_exchange(&bench->max_block_time, &max_time, elapsed_time))
            break;
    }

    atomic_fetch_add(&bench->total_block_time, elapsed_time);
    atomic_fetch_add(&bench->block_count, 1);
}

static void benchmark_proc_dummy_fun(uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1, void* ctx)
{
    UNUSED_PARAM(x0);
    UNUSED_PARAM(x1);
    UNUSED_PARAM(y0);
    UNUSED_PARAM(y1);
    UNUSED_PARAM(ctx);
}

void benchmark_create(benchmark** pbench, uint32_t runs, mdb_kernel* kernel,
                      struct rsched* sched)
{
    benchmark* bench;

    *pbench = (benchmark*)calloc(1, sizeof(benchmark));
    bench = *pbench;

    bench->kernel = kernel;
    bench->sched = sched;

    bench->runs = runs;

    atomic_store(&bench->total_block_time, 0);
    atomic_store(&bench->min_block_time, UINT64_MAX);
    atomic_store(&bench->max_block_time, 0);
    atomic_store(&bench->block_count, 0);

    perf_timer_init(&bench->tm_kernel);

    rsched_set_user_context(bench->sched, &benchmark_proc_fun, bench);
}

void benchmark_destroy(benchmark* bench)
{
    free(bench);
}

static void benchmark_run_kernel(benchmark* bench)
{
    uint32_t runs = bench->runs;
    uint32_t run = 0;

    perf_timer_start(&bench->tm_kernel);

    while(run < runs)
    {
        rsched_host_yield(bench->sched);
        rsched_requeue(bench->sched);

        ++run;
    }

    perf_timer_stop(&bench->tm_kernel);
}

void benchmark_run(benchmark* bench)
{
    benchmark_run_kernel(bench);
}

void benchmark_print_summary(benchmark* bench)
{
    uint32_t threads = rsched_threads_count(bench->sched);

    double block_ms = ns_to_ms(atomic_load(&bench->total_block_time) / threads);
    double min_block_ms = ns_to_ms(atomic_load(&bench->min_block_time));
    double max_block_ms = ns_to_ms(atomic_load(&bench->max_block_time));

    uint64_t block_count = atomic_load(&bench->block_count);
    double total_exec_time = perf_timer_diff_sec(&bench->tm_kernel);

    PARAM_INFO("Total blocks", "%lu", block_count);
    PARAM_INFO("Avg block time", "%f ms", (block_ms / block_count));
    PARAM_INFO("Min block time", "%f ms", min_block_ms);
    PARAM_INFO("Max block time", "%f ms", max_block_ms);
    PARAM_INFO("Total block time", "%f sec", (block_ms / 1000.0));
    PARAM_INFO("Total execution time", "%f sec", total_exec_time);
    PARAM_INFO("Total runs", "%i", bench->runs);
    PARAM_INFO("Avg FPS", "%f", ((double)bench->runs / total_exec_time));
}