#pragma once

#include <string.h>
#include <stdint.h>
#include <time.h>
#include <mdb/tools/compiler.h>
#include <stdio.h>

#define NANOSECONDS_IN_SECOND       UINT64_C(1000000000)
#define NANOSECONDS_IN_MILLISECOND  UINT64_C(1000000)
#define NANOSECONDS_IN_MICROSECOND  UINT64_C(1000)

__always_inline static double ns_to_sec(uint64_t ns)
{
    return (double)ns / NANOSECONDS_IN_SECOND;
}

__always_inline static double ns_to_ms(uint64_t ns)
{
    return (double)ns / NANOSECONDS_IN_MILLISECOND;
}

__always_inline static double timespec_get_total_sec(const struct timespec* ts)
{
    double total_sec;

    if (ts->tv_sec == 0)
        return (double) ts->tv_nsec / NANOSECONDS_IN_SECOND;

    total_sec = (double) ts->tv_nsec / NANOSECONDS_IN_SECOND;
    total_sec += (double) ts->tv_sec;

    return total_sec;
}


__always_inline static double sample_timer(void)
{
    struct timespec tm;

    clock_gettime(CLOCK_MONOTONIC, &tm);

    return timespec_get_total_sec(&tm);

}

__always_inline static uint64_t timespec_get_total_ns(const struct timespec* ts)
{
    uint64_t total_ns;

    if (ts->tv_sec == 0)
        return (uint64_t)ts->tv_nsec;

    total_ns = (uint64_t) ts->tv_sec * NANOSECONDS_IN_SECOND;
    total_ns += ts->tv_nsec;

    return total_ns;
}

typedef struct
{
    struct timespec start, end;

} perf_timer;

__always_inline static void perf_timer_init(perf_timer* tm)
{
    memset(tm, 0, sizeof(perf_timer));
}

__always_inline static void perf_timer_start(perf_timer* tm)
{
    clock_gettime(CLOCK_MONOTONIC, &tm->start);
}

__always_inline static void perf_timer_stop(perf_timer* tm)
{
    clock_gettime(CLOCK_MONOTONIC, &tm->end);
}

__always_inline static uint64_t perf_timer_diff_ns(const perf_timer* tm)
{
    uint64_t total_start = timespec_get_total_ns(&tm->start);
    uint64_t total_end   = timespec_get_total_ns(&tm->end);

    uint64_t diff = total_end - total_start;

    return diff;
}

__always_inline static double perf_timer_diff_sec(const perf_timer* tm)
{
    double total_start  = timespec_get_total_sec(&tm->start);
    double total_end    = timespec_get_total_sec(&tm->end);

    double diff = total_end - total_start;

    return diff;
}

static inline void perf_format_time(uint64_t ns, char* buf, size_t sz)
{
    if(ns < 10 * NANOSECONDS_IN_MICROSECOND)
    {
        snprintf(buf, sz, "%04lu ns", ns);
    }
    else
    if(ns < 10 * NANOSECONDS_IN_MILLISECOND)
    {
        snprintf(buf, sz, "%04lu mc", ns / NANOSECONDS_IN_MICROSECOND);
    }
    else
    if(ns < 10 * NANOSECONDS_IN_SECOND)
    {
        snprintf(buf, sz, "%04lu ms", ns / NANOSECONDS_IN_MILLISECOND);
    }
    else
    {
        double d = (double)ns / NANOSECONDS_IN_SECOND;
        snprintf(buf, sz, "%.2f sc", d);
    }
}
