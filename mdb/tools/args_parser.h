#pragma once

enum
{
    MODE_ONESHOT,
    MODE_BENCHMARK,
    MODE_RENDER
};

struct block_size
{
    int x, y;
};

/* Used by main to communicate with parse_opt. */
struct arguments
{
    int width, height;
    int bailout;
    struct block_size block_size;
    int kernel_type;
    int threads;
    int mode;
    int benchmark_runs;
    int silent, verbose;
    char* output_file;
};

void args_parse(int argc, char** argv, struct arguments* arguments);