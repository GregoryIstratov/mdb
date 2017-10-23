#include <tools/log.h>
#include "rsched_profile.h"
#include "rsched.h"

#if defined(CONFIG_RSCHED_PROFILE)
void rsched_profile_stat_init(struct profile_stat* stat,
                              struct rsched_profile_hist_options* opts)
{
        stat->max = 0;
        stat->min = UINT64_MAX;
        stat->total = 0;

        perf_hist_init(&stat->hist,
                       opts->size,
                       opts->min,
                       opts->max,
                       opts->log_scale);
}

void rsched_profile_stat_destroy(struct profile_stat* stat)
{
        perf_hist_destroy(&stat->hist);
}

void rsched_profile_init(struct profile_stats* stats,
                         struct rsched_profile_options* opts)
{
        rsched_profile_stat_init(&stats->run, &opts->run_hist);
        rsched_profile_stat_init(&stats->task, &opts->task_hist);
        rsched_profile_stat_init(&stats->payload, &opts->payload_hist);
}

void rsched_profile_destroy(struct profile_stats* stats)
{
        rsched_profile_stat_destroy(&stats->run);
        rsched_profile_stat_destroy(&stats->task);
        rsched_profile_stat_destroy(&stats->payload);
}


static
void print_worker_stats(struct worker_stats* stats)
{
        struct profile_stats* profile;
        uint64_t task_time_avg;
        uint64_t payload_avg;
        uint64_t overhead_total;
        int64_t overhead_avg, overhead_min, overhead_max;

        profile = &stats->profile;

        task_time_avg = profile->task.total / MAX(stats->task_count, 1);
        payload_avg = profile->payload.total / MAX(stats->task_count, 1);
        overhead_total = profile->task.total - profile->payload.total;
        overhead_avg = task_time_avg - payload_avg;
        overhead_min = profile->task.min - profile->payload.min;
        overhead_max = profile->task.max - profile->payload.max;

        PARAM_INFO("Run time", "%'lu ns", profile->run.total);
        PARAM_INFO("Task count", "%'lu", stats->task_count);
        PARAM_INFO("Task total", "%'lu ns", profile->task.total);
        PARAM_INFO("Task avg", "%'lu ns", task_time_avg);
        PARAM_INFO("Task min", "%'lu ns", profile->task.min);
        PARAM_INFO("Task max", "%'lu ns", profile->task.max);
        PARAM_INFO("Payload total", "%'lu ns", profile->payload.total);
        PARAM_INFO("Payload avg", "%'lu ns", payload_avg);
        PARAM_INFO("Payload min", "%'lu ns", profile->payload.min);
        PARAM_INFO("Payload max", "%'lu ns", profile->payload.max);
        PARAM_INFO("Overhead total", "%'lu ns", overhead_total);
        PARAM_INFO("Overhead avg", "%'ld ns", overhead_avg);
        PARAM_INFO("Overhead min", "%'ld ns", overhead_min);
        PARAM_INFO("Overhead max", "%'ld ns", overhead_max);
}


static
void print_workers_run_hist(struct rsched* sched)
{
        uint32_t i;

        LOG_SAY("==============================================");
        LOG_SAY("Worker [host] run time histogram");

        perf_hist_print(&sched->host_stats.profile.run.hist);

        for(i = 0; i < sched->n_workers; ++i)
        {
                LOG_SAY("Worker [%d] run time histogram", i);
                perf_hist_print(&sched->worker[i].stats.profile.run.hist);
        }

}

static
void print_workers_task_hist(struct rsched* sched)
{
        uint32_t i;

        LOG_SAY("==============================================");
        LOG_SAY("Worker [host] task time histogram");

        perf_hist_print(&sched->host_stats.profile.task.hist);

        for(i = 0; i < sched->n_workers; ++i)
        {
                LOG_SAY("Worker [%d] task time histogram", i);
                perf_hist_print(&sched->worker[i].stats.profile.task.hist);
        }

}

static
void print_workers_payload_hist(struct rsched* sched)
{
        uint32_t i;

        LOG_SAY("==============================================");
        LOG_SAY("Worker [host] payload time histogram");

        perf_hist_print(&sched->host_stats.profile.payload.hist);

        for(i = 0; i < sched->n_workers; ++i)
        {
                LOG_SAY("Worker [%d] payload time histogram", i);
                perf_hist_print(&sched->worker[i].stats.profile.payload.hist);
        }

}

void rsched_print_stats(struct rsched* sched)
{
        uint32_t i;

        LOG_SAY("==============================================");
        LOG_SAY("**************** RSCHED STATS ****************");
        LOG_SAY("==============================================");

        LOG_SAY("Worker [Host] summary");
        print_worker_stats(&sched->host_stats);

        for(i = 0; i < sched->n_workers; ++i)
        {
                LOG_SAY("Worker [%d] summary", i);
                print_worker_stats(&sched->worker[i].stats);
        }

        if(sched->stats.run_time_hist_show)
                print_workers_run_hist(sched);


        if(sched->stats.task_time_hist_show)
                print_workers_task_hist(sched);

        if(sched->stats.payload_hist_show)
                print_workers_payload_hist(sched);

        LOG_SAY("==============================================");
}

#endif /* CONFIG_RSCHED_PROFILE */