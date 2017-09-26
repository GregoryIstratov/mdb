#pragma once

#include <string.h>
#include <stdint.h>
#include <mdb/tools/utils.h>
#include <mdb/tools/log.h>

#define NANOSECONDS_IN_SECOND 1000000000
#define NANOSECONDS_IN_MILLISECOND 1000000

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
    if (ts->tv_sec == 0)
        return (double) ts->tv_nsec / NANOSECONDS_IN_SECOND;

    double total_sec;
    total_sec = (double) ts->tv_nsec / NANOSECONDS_IN_SECOND;
    total_sec += (double) ts->tv_sec;

    return total_sec;
}


__always_inline static double sample_timer(void)
{
    struct timespec tm;

    if(clock_gettime(CLOCK_MONOTONIC, &tm))
    {
        LOG_ERROR("Failed to take the time and switched to fallback");
    }

    return timespec_get_total_sec(&tm);

}

__always_inline static uint64_t timespec_get_total_ns(const struct timespec* ts)
{
    if (ts->tv_sec == 0)
        return (uint64_t)ts->tv_nsec;

    uint64_t total_ns = (uint64_t) ts->tv_sec * NANOSECONDS_IN_SECOND;
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
    if(clock_gettime(CLOCK_MONOTONIC, &tm->start))
    {
        LOG_WARN("Failed to take the time and switched to fallback");
    }
}

__always_inline static void perf_timer_stop(perf_timer* tm)
{
    if(clock_gettime(CLOCK_MONOTONIC, &tm->end))
    {
        LOG_WARN("Failed to take the time and switched to fallback");
    }
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