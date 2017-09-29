#pragma once

#include <stdint.h>
#include <mdb/sched/rsched.h>

enum
{
    MODE_ONESHOT,
    MODE_BENCHMARK,
    MODE_RENDER
};


/* Used by main to communicate with parse_opt. */
struct arguments
{
    uint32_t width, height;
    uint32_t bailout;
    struct block_size block_size;
    char* kernel_name;
    int threads;
    int mode;
    int benchmark_runs;
    int silent, verbose;
    char* output_file;
};

void args_parse(int argc, char** argv, struct arguments* arguments);