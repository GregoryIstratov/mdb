#pragma once

#include <string.h>
#include <stdint.h>
#include <time.h>
#include <mdb/tools/compiler.h>
#include <stdio.h>
#include <inttypes.h>

#define NS_IN_SEC UINT64_C(1000000000)
#define NS_IN_MS  UINT64_C(1000000)
#define NS_IN_MCS UINT64_C(1000)

static inline
double ns_to_sec(uint64_t ns)
{
        return (double)ns / NS_IN_SEC;
}

static inline
double ns_to_ms(uint64_t ns)
{
        return (double)ns / NS_IN_MS;
}

static inline
double timespec_get_total_sec(const struct timespec* ts)
{
        double total_sec;

        if (ts->tv_sec == 0)
                return (double) ts->tv_nsec / NS_IN_SEC;

        total_sec = (double) ts->tv_nsec / NS_IN_SEC;
        total_sec += (double) ts->tv_sec;

        return total_sec;
}


static inline
double sample_timer(void)
{
        struct timespec tm;

        clock_gettime(CLOCK_MONOTONIC, &tm);

        return timespec_get_total_sec(&tm);

}

static inline
uint64_t timespec_get_total_ns(const struct timespec* ts)
{
        uint64_t total_ns;

        if (ts->tv_sec == 0)
                return ts->tv_nsec;

        total_ns = ts->tv_sec * NS_IN_SEC;
        total_ns += ts->tv_nsec;

        return total_ns;
}

struct perf_timer
{
        struct timespec start, end;

};

static inline
void perf_timer_start(struct perf_timer* tm)
{
        clock_gettime(CLOCK_MONOTONIC, &tm->start);
}

static inline
void perf_timer_stop(struct perf_timer* tm)
{
        clock_gettime(CLOCK_MONOTONIC, &tm->end);
}

static inline
uint64_t perf_timer_diff_ns(const struct perf_timer* tm)
{
        uint64_t total_start = timespec_get_total_ns(&tm->start);
        uint64_t total_end   = timespec_get_total_ns(&tm->end);

        uint64_t diff = total_end - total_start;

        return diff;
}

static inline
double perf_timer_diff_sec(const struct perf_timer* tm)
{
        double total_start  = timespec_get_total_sec(&tm->start);
        double total_end    = timespec_get_total_sec(&tm->end);

        double diff = total_end - total_start;

        return diff;
}

static inline
void perf_format_time(uint64_t ns, char* buf, size_t sz)
{
        if(ns < 10 * NS_IN_MCS)
        {
                snprintf(buf, sz, "%" PRId64 " ns", ns);
        }
        else
        if(ns < 10 * NS_IN_MS)
        {
                snprintf(buf, sz, "%04" PRId64 " mc", ns / NS_IN_MCS);
        }
        else
        if(ns < 10 * NS_IN_SEC)
        {
                snprintf(buf, sz, "%04" PRId64 " ms", ns / NS_IN_MS);
        }
        else
        {
                double d = (double)ns / NS_IN_SEC;
                snprintf(buf, sz, "%.2f sc", d);
        }
}
