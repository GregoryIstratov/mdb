#include "rsched.h"

#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <errno.h>
#include <mdb/tools/compiler.h>
#include <mdb/tools/atomic.h>
#include <mdb/tools/log.h>
#include <mdb/tools/error_codes.h>
#include <mdb/tools/timer.h>
#include <mdb/tools/hist.h>

#include "rsched_queue.h"
#include "rsched_worker.h"
#include "rsched_common.h"


static inline
int rsched_wait_workers(struct rsched* sched);

static
void rsched_init_structure(struct rsched** psched, struct rsched_options* opts)
{
        struct rsched* sched;
        uint32_t workers = opts->threads - 1;

        *psched = calloc(1, sizeof(**psched));
        sched = *psched;

        sched->worker       = calloc(workers, sizeof(*sched->worker));
        sched->n_workers    = workers;
        sched->user_fun     = NULL;
        sched->user_ctx     = NULL;

        rsched_worker_init_stats(&sched->host_stats, opts);

#if defined(CONFIG_RSCHED_PROFILE)
        sched->stats.run_time_hist_show = opts->profile.run_hist.show;
        sched->stats.task_time_hist_show = opts->profile.task_hist.show;
        sched->stats.payload_hist_show = opts->profile.payload_hist.show;
#endif
}

int rsched_create(struct rsched** psched, struct rsched_options* opts)
{
        uint32_t i;
        struct rsched* sched;
        uint32_t workers = opts->threads - 1;

        rsched_init_structure(psched, opts);
        sched = *psched;

        rsched_queue_init(&sched->queue);

        for(i = 0; i < workers; ++i)
        {
                int ret = rsched_worker_init(&sched->worker[i],
                                             i,
                                             &sched->queue,
                                             opts);

                if(ret != MDB_SUCCESS)
                        goto shutdown_ret_fail;
        }

        if(rsched_wait_workers(sched) != MDB_SUCCESS)
        {
                LOG_ERROR("Failed to sync workers.");
                goto shutdown_ret_fail;
        }

        return MDB_SUCCESS;

shutdown_ret_fail:
        rsched_shutdown(sched);
        *psched = NULL;

        return MDB_FAIL;
}

static
int bind_thread_to_cpu(pthread_t tid, uint32_t cpu_id)
{
        cpu_set_t cpu;
        int ret;

        CPU_ZERO(&cpu);

        CPU_SET(cpu_id, &cpu);

        ret = pthread_setaffinity_np(tid, sizeof(cpu), &cpu);
        if(ret != 0)
        {
                LOG_WARN("Cannot set thread affinity: %s", strerror(ret));
                return MDB_FAIL;
        }

        return MDB_SUCCESS;

}

int rsched_tune_thread_affinity(struct rsched* sched)
{
        uint32_t i;
        bool errs = false;

        /* bind a host thread to the first cpu */
        if(bind_thread_to_cpu(pthread_self(), 0) == MDB_SUCCESS)
                LOG_VINFO(LOG_VERBOSE1, "Host thread bind to cpu 0");
        else
                errs = true;


        for(i = 0; i < sched->n_workers; ++i)
        {
                int ret = bind_thread_to_cpu(sched->worker[i].pthr_id, i + 1);
                if(ret == MDB_SUCCESS)
                        LOG_VINFO(LOG_VERBOSE1,
                                  "Worker [%d] thread bind to cpu %d",
                                  i, i + 1);
                else
                        errs = true;
        }


        if(errs)
                LOG_WARN("Setting threads affinity was not successful. "
                         "This may lead to performance issues "
                         "or program may not work properly");


        return MDB_SUCCESS;
}

static inline
void update_workers_user_ctx(struct rsched* sched,
                             rsched_user_fun fun, void* user_ctx)
{
        uint32_t i;
        for(i = 0; i < sched->n_workers; ++i)
        {
                pthread_spin_lock(&sched->worker[i].lock);

                sched->worker[i].user_fun = fun;
                sched->worker[i].user_ctx = user_ctx;

                pthread_spin_unlock(&sched->worker[i].lock);
        }
}

void rsched_set_user_context(struct rsched* sched,
                             rsched_user_fun fun, void* user_ctx)
{
        sched->user_fun = fun;
        sched->user_ctx = user_ctx;

        update_workers_user_ctx(sched, fun, user_ctx);
}

static
void rsched_destroy_workers(struct rsched* sched)
{
        uint32_t i;
        for(i = 0; i < sched->n_workers; ++i)
        {
                rsched_worker_destroy(&sched->worker[i]);
        }
}

static
void rsched_destroy_structure(struct rsched* sched)
{
        free(sched->worker);
        free(sched);
}

void rsched_shutdown(struct rsched* sched)
{
        rsched_worker_destroy_stats(&sched->host_stats);

        rsched_destroy_workers(sched);

        rsched_queue_destroy(&sched->queue);

        rsched_destroy_structure(sched);
}

uint32_t rsched_threads_count(struct rsched* sched)
{
        return sched->n_workers + 1;
}

void rsched_requeue(struct rsched* sched)
{
        rsched_queue_requeue(&sched->queue);
}

static inline
int rsched_wait_worker(struct rsched_worker* worker)
{
        int state;
        while(1)
        {
                state = rsched_get_worker_state(worker);
                switch(state)
                {
                case RS_ST_WAITING:
                        return MDB_SUCCESS;

                default:
                        rsched_yield_cpu();
                        break;

                case RS_ST_DOWN:
                        LOG_ERROR("Worker [%d] is down.", worker->id);
                        return MDB_FAIL;
                }
        }
}

static inline
int rsched_wait_workers(struct rsched* sched)
{
        uint32_t i;
        int ret;
        for(i = 0; i < sched->n_workers; ++i)
        {
                ret = rsched_wait_worker(&sched->worker[i]);
                if(ret != MDB_SUCCESS)
                        return ret;

        }

        return MDB_SUCCESS;
}

static inline
void rsched_run_workers(struct rsched* sched)
{
        uint32_t i;

        for(i = 0; i < sched->n_workers; ++i)
        {
                rsched_send_worker_sig(&sched->worker[i], RS_SIG_START);
        }
}

int rsched_host_yield(struct rsched* sched)
{
        rsched_user_fun proc_fun;
        void* user_ctx;
        struct worker_stats* stats = &sched->host_stats;

        user_ctx = sched->user_ctx;
        proc_fun = sched->user_fun;
        if(proc_fun == NULL)
        {
                LOG_ERROR("Host worker."
                          " Process function is not set. Exiting...");

                return MDB_FAIL;
        }

        rsched_run_workers(sched);

        rsched_profile_start(&stats->profile.run);
        for (;;)
        {
                struct rsched_task* t;

                rsched_profile_start(&stats->profile.task);

                t = rsched_queue_pop(&sched->queue);

                if (t == NULL)
                {
                        rsched_profile_stop(&stats->profile.task);
                        break;
                }

                rsched_profile_start(&stats->profile.payload);

                proc_fun(t->x0, t->x1, t->y0, t->y1, user_ctx);

                rsched_profile_stop(&stats->profile.payload);

                ++stats->task_count;

                rsched_profile_stop(&stats->profile.task);
        }
        rsched_profile_stop(&stats->profile.run);;

        if(rsched_wait_workers(sched) != MDB_SUCCESS)
        {
                LOG_ERROR("Failed to sync workers.");
                return MDB_FAIL;
        }

        return MDB_SUCCESS;
}

void rsched_create_tasks(struct rsched* sched, uint32_t width, uint32_t height,
                         struct block_size* grain)
{
        uint32_t wxh = width * height;
        uint32_t grain2 = grain->x * grain->y;
        uint32_t qlen = wxh / grain2 + (wxh % grain2 != 0);

        rsched_queue_resize(&sched->queue,
                            qlen,
                            RS_QUE_DISCARD
                            | RS_QUE_ZERO);

        rsched_split_task(&sched->queue, 0, width-1, 0, height-1, grain);
}
