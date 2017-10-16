#include <mdb/tools/hist.h>
#include <mdb/tools/timer.h>
#include <mdb/tools/error_codes.h>
#include "rsched_worker.h"
#include "rsched_queue.h"


static void* rsched_worker(void* arg);

void rsched_worker_init_stats(struct worker_stats* stats,
                              struct rsched_options* opts)
{
        stats->task_count = 0;

        rsched_profile_init(&stats->profile, &opts->profile);

}

void rsched_worker_destroy_stats(struct worker_stats* stats)
{
        rsched_profile_destroy(&stats->profile);
}

void rsched_worker_destroy(struct rsched_worker* worker)
{
        int state = rsched_get_worker_state(worker);

        RSCHED_DEBUG("Worker [%d] state = %d", worker->id, state);

        if(state != RS_ST_DOWN)
        {
                rsched_send_worker_sig(worker, RS_SIG_QUIT);
        }

        RSCHED_DEBUG("Joining worker [%d] thread...", worker->id);

        pthread_join(worker->pthr_id, NULL);

        pthread_spin_destroy(&worker->lock);

        rsched_worker_destroy_stats(&worker->stats);

        RSCHED_DEBUG("Worker [%d] has been destroyed.", worker->id);
}

int rsched_worker_init(struct rsched_worker* worker, uint32_t id,
                       struct rsched_queue* queue,
                       struct rsched_options* opts)
{
        static const size_t name_size = 32;

        char name[name_size];
        int ret;

        LOG_VINFO(LOG_VERBOSE1, "Initializing worker [%d]...", id);

        rsched_worker_init_stats(&worker->stats, opts);

        worker->queue = queue;
        worker->id = id;

        pthread_spin_init(&worker->lock, 0);

        RSCHED_DEBUG("Creating worker[%d] thread...", id);

        ret = pthread_create(&worker->pthr_id,
                             NULL,
                             &rsched_worker,
                             worker);
        if(ret)
        {
                LOG_ERROR("[pthread_create]: %s", strerror(ret));

                atomic_store(&worker->state, RS_ST_DOWN);
                return MDB_FAIL;
        }


        snprintf(name, name_size, "worker%d", id);
        ret = pthread_setname_np(worker->pthr_id, name);
        if(ret)
        {
                LOG_WARN("[pthread_setname_np]: %s", strerror(ret));
        }

        return MDB_SUCCESS;
}



__hot static
int rsched_worker_loop(struct rsched_worker* worker,
                       rsched_user_fun proc_fun, void* user_ctx)
{
        struct rsched_task* task;

        int sig;

        rsched_profile_start(&worker->stats.profile.run);

        while(1)
        {
                rsched_profile_start(&worker->stats.profile.task);

                task = rsched_queue_pop(worker->queue);

                if(task == NULL)
                {
                        sig = RS_SIG_NONE;
                        rsched_worker_sig_unlock(worker);
                        goto loop_exit;
                }

                rsched_profile_start(&worker->stats.profile.payload);

                proc_fun(task->x0, task->x1, task->y0, task->y1, user_ctx);

                ++worker->stats.task_count;

                rsched_profile_stop(&worker->stats.profile.payload);

                sig = rsched_worker_sig_lock_receive(worker);

                switch(sig)
                {
                case RS_SIG_NONE:
                case RS_SIG_START:
                {
                        rsched_worker_sig_unlock(worker);
                        rsched_profile_stop(&worker->stats.profile.task);
                        continue;
                }

                case RS_SIG_INT:
                case RS_SIG_QUIT:
                default:
                        goto loop_exit;
                }
        }

loop_exit:
        rsched_profile_stop(&worker->stats.profile.task);
        rsched_profile_stop(&worker->stats.profile.run);

        return sig;
}

static
void* rsched_worker(void* arg)
{
        struct rsched_worker* worker = arg;
        rsched_user_fun proc_fun;
        void* user_ctx;
        uint32_t worker_id = worker->id;

        int sig;

        RSCHED_DEBUG("Worker [%d] initialization state. Yielding...", worker_id);

        goto worker_yield;

worker_loop:
        sig = rsched_worker_loop(worker, proc_fun, user_ctx);

        if(sig != RS_SIG_NONE)
                goto sig_handle;

worker_yield:
        sig = rsched_worker_yield(worker);

sig_handle:
        if(likely(sig == RS_SIG_START))
        {
                RSCHED_DEBUG("Worker [%d] SIG_START.", worker_id);

                pthread_spin_lock(&worker->lock);
                user_ctx = worker->user_ctx;
                proc_fun = worker->user_fun;
                pthread_spin_unlock(&worker->lock);

                if(proc_fun == NULL)
                {
                        LOG_ERROR("Worker [%d]. "
                                  "Process function is not set. "
                                  "Exiting...",
                                  worker_id);

                        goto worker_exit;
                }

                RSCHED_DEBUG("Worker [%d] Setting state to RUNNING...",
                             worker_id);

                rsched_worker_set_state(worker, RS_ST_RUNNING);

                RSCHED_DEBUG("Worker [%d] Unlocking signals...", worker_id);

                rsched_worker_sig_unlock(worker);

                goto worker_loop;
        }
        else if(sig == RS_SIG_QUIT)
        {
                RSCHED_DEBUG("Worker [%d] SIG_QUIT.", worker_id);

                goto worker_exit;
        }
        else if(sig == RS_SIG_INT)
        {
                RSCHED_DEBUG("Worker [%d] SIG_INT.", worker_id);

                RSCHED_DEBUG("Worker [%d] Unlocking signals...", worker_id);

                rsched_worker_sig_unlock(worker);

                goto worker_yield;
        }
        else
        {
                LOG_ERROR("Worker [%d] Unknown SIG = %d. Exiting...",
                          worker_id, sig);

                goto worker_exit;
        }


worker_exit:
        LOG_VINFO(LOG_VERBOSE1, "Worker [%d] exiting...", worker_id);

        RSCHED_DEBUG("Worker [%d] Setting state to DOWN...", worker_id);

        rsched_worker_set_state(worker, RS_ST_DOWN);

        RSCHED_DEBUG("Worker [%d] Unlocking signals...", worker_id);

        rsched_worker_sig_unlock(worker);

        RSCHED_DEBUG("Worker [%d] exit thread.", worker_id);

        return NULL;
}
