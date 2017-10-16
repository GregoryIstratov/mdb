#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <mdb/config/config.h>

enum
{
        MODE_ONESHOT,
        MODE_BENCHMARK,
        MODE_RENDER
};

struct optional_bool
{
        bool value;
        bool set;
};

struct optional_u64
{
        uint64_t value;
        bool set;
};

struct optional_u32
{
        uint32_t value;
        bool set;
};

#define optional_get(parg, _default) \
        ((parg)->set ? (parg)->value : (_default))

#define optional_set(parg, _value) \
        do { (parg)->value = (_value); (parg)->set = true; } while(0)

struct arg_rsched_hist
{
        struct optional_bool show;
        struct optional_bool log_scale;
        struct optional_u32  size;
        struct optional_u64  min, max;
};

struct arg_rsched
{
        /* rsched profile options */
        struct arg_rsched_hist run_hist;

        struct arg_rsched_hist task_hist;

        struct arg_rsched_hist payload_hist;
};

struct arguments
{
        uint32_t width, height;
        uint32_t bailout;
        uint32_t block_size_x;
        uint32_t block_size_y;
        char* kernel_name;
        int threads;
        int mode;
        int benchmark_runs;
        int silent, verbose;
        char* output_file;
        int shader_colors;

#if defined(CONFIG_RSCHED_PROFILE)
        struct arg_rsched rsched;

#endif
};

void args_parse(int argc, char** argv, struct arguments* arguments);
