#pragma once

#include <mdb/config/config.h>
#include <stdint.h>
#include <mdb/tools/timer.h>
#include <mdb/tools/hist.h>

#define sample_max_time(vptr, sample) \
        do { if(*(vptr) < (sample)) *(vptr) = sample; } while(0)

#define sample_min_time(vptr, sample) \
        do { if(*(vptr) > (sample)) *(vptr) = sample; } while(0)

#ifdef CONFIG_RSCHED_PROFILE
struct rsched_profile_hist_options
{
        bool show;
        bool log_scale;
        uint32_t size;
        uint64_t min, max;
};

struct rsched_profile_options
{
        struct rsched_profile_hist_options run_hist;
        struct rsched_profile_hist_options task_hist;
        struct rsched_profile_hist_options payload_hist;
};

struct profile_stat
{
        uint64_t total;
        uint64_t max;
        uint64_t min;
        struct perf_hist hist;
        perf_timer tm;
};

struct rsched_stats
{
        bool run_time_hist_show;
        bool task_time_hist_show;
        bool payload_hist_show;
};

struct profile_stats;

void rsched_profile_init(struct profile_stats* stats,
                         struct rsched_profile_options* opts);

void rsched_profile_destroy(struct profile_stats* stats);

void rsched_profile_stat_init(struct profile_stat* stat,
                              struct rsched_profile_hist_options* opts);

void rsched_profile_stat_destroy(struct profile_stat* stat);

static inline
void rsched_profile_start(struct profile_stat* stat)
{
        perf_timer_start(&stat->tm);
}


static inline
void rsched_profile_stop(struct profile_stat* stat)
{
        uint64_t time;

        perf_timer_stop(&stat->tm);
        time = perf_timer_diff_ns(&stat->tm);

        sample_max_time(&stat->max, time);
        sample_min_time(&stat->min, time);

        perf_hist_add(&stat->hist, time);

        stat->total += time;
}

struct rsched;
void rsched_print_stats(struct rsched* sched);

#else
struct rsched_profile_hist_options {};
struct rsched_profile_options {};
struct profile_stat {};
struct rsched_stats {};

#define rsched_profile_start(stat) UNUSED_PARAM(stat)
#define rsched_profile_stop(stat) UNUSED_PARAM(stat)

#define rsched_profile_init(stats,opts) UNUSED_PARAM(stats); UNUSED_PARAM(opts)
#define rsched_profile_destroy(stats) UNUSED_PARAM(stats)

#define rsched_print_stats(sched) UNUSED_PARAM(sched)

#endif

struct profile_stats
{
        struct profile_stat run;
        struct profile_stat task;
        struct profile_stat payload;
};
