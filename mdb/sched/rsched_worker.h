#pragma once

#include <stdint.h>
#include <pthread.h>
#include <mdb/tools/atomic.h>
#include <mdb/tools/hist.h>
#include <mdb/tools/timer.h>

#include "rsched_common.h"
#include "rsched_queue.h"
#include "rsched_profile.h"

enum
{
        /* Worker states */

        RS_ST_RUNNING   = 1,
        RS_ST_WAITING   = 2,
        RS_ST_DOWN      = -1,


        /* Worker signals */

        RS_SIG_NONE     = 0,
        RS_SIG_START    = 1,
        RS_SIG_INT      = 3,
        RS_SIG_QUIT     = 4,

        RS_SIG_LOCKED   = -1,

        RS_LAST

};

struct worker_stats
{
        uint64_t task_count;

        struct profile_stats profile;
};

struct rsched_worker
{
        /* Pointer to the shared queue
         * If an update required ensure worker is in a waiting state.
         */
        struct rsched_queue* queue;

        pthread_spinlock_t lock;

        /* Shared data, read/write on lock */
        rsched_user_fun user_fun;
        void* user_ctx;

        /* --------------------------- */

        struct worker_stats stats;

        uint32_t id;

        pthread_t pthr_id;

        __cache_aligned
        __atomic
        int signal;

        __cache_aligned
        __atomic
        int state;
};

/*
 * Worker initialization and deinitialization
 */

int rsched_worker_init(struct rsched_worker* worker, uint32_t id,
                       struct rsched_queue* queue,
                       struct rsched_options* opts);

void rsched_worker_destroy(struct rsched_worker* worker);

/*
 * Worker stats
 */

void rsched_worker_init_stats(struct worker_stats* stats,
                              struct rsched_options* opts);

void rsched_worker_destroy_stats(struct worker_stats* stats);


/* These functions are for communicating host and workers.
 * The communication goes through signals and states.
 *
 * A state of a worker can be changed only by its own worker.
 *
 * Host can only send a signal and read a state of a worker.
 *
 * While there's a pending signal to a worker, sending another signal
 * will block the sender thread until the signal is not received by the worker
 * and worker has not changed its state.
 *
 * Reading a worker's state also blocks the reading thread until there's
 * a pending signal.
 */

static inline
int rsched_worker_sig_lock_receive(struct rsched_worker* worker)
{
        return atomic_exchange(&worker->signal,
                               RS_SIG_LOCKED);
}

static inline
void rsched_worker_sig_unlock(struct rsched_worker* worker)
{
        atomic_store(&worker->signal, RS_SIG_NONE);
}


static inline
void rsched_send_worker_sig(struct rsched_worker* worker, int sig)
{
        int esig = RS_SIG_NONE;

        while(!atomic_compare_exchange(&worker->signal,
                                       &esig,
                                       sig))
        {
                rsched_yield_cpu();
        }
}

static inline
int rsched_get_worker_state(struct rsched_worker* worker)
{
        /*
         * Don't allow reading a worker state while there's a pending signal.
         */
        while(atomic_load(&worker->signal) != RS_SIG_NONE)
        {
                rsched_yield_cpu();
        }

        return atomic_load(&worker->state);
}

static inline
void rsched_worker_set_state(struct rsched_worker* worker, int state)
{
        atomic_store(&worker->state, state);
}


static inline
int rsched_worker_yield(struct rsched_worker* worker)
{
        int sig;

        rsched_worker_set_state(worker, RS_ST_WAITING);

        sig = rsched_worker_sig_lock_receive(worker);
        while (sig == RS_SIG_NONE)
        {
                rsched_worker_sig_unlock(worker);

                rsched_yield_cpu();

                sig = rsched_worker_sig_lock_receive(worker);
        }

        return sig;
}
