#pragma once

#include <stdint.h>
#include <stdbool.h>

struct perf_hist_bin
{
        uint64_t data;
        uint64_t min, max;
};

struct perf_hist
{
        uint32_t size;
        struct perf_hist_bin* bin;
};

void perf_hist_init(struct perf_hist* hist, uint32_t size,
                    uint64_t min, uint64_t max, bool log_scale);

void perf_hist_destroy(struct perf_hist* hist);

void perf_hist_add(struct perf_hist* hist, uint64_t val);

void perf_hist_print(struct perf_hist* hist);
