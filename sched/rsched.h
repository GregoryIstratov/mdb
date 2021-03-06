#pragma once

/* This is a high performance scheduler for dispatching render tasks.
 *
 * The scheduler uses a fixed length shared between workers queue, once created
 * it cannot be changed during a processing stage, this allows using
 * simple and fast algorithm to retrieve tasks from the queue without any locks
 * only one atomic compare and swap operation is involved.
 * This approach combined with a good chosen grain of tasks gives a very good
 * uniform load balancing across CPU cores, so none of cores would stall
 * without a job.
 *
 * The scheduler creates given as a thread parameter n - 1 workers, one worker
 * is always run on the host thread. Scheduler automatically binds each worker
 * including host one to each cpu/core, for preventing cpu migrations and
 * improving cache coherency, on some hosts this operation might require
 * specific user privileges.
 *
 * The scheduler is designed to work with rendering tasks, but technically can
 * work with any computation tasks.
 * The tasks is a small blocks of NxM size, which are generated by splitting
 * the input surface's rectangle.
 *
 * There are a few functions which user should use to control scheduler.
 * Besides creation/destroying and task splitting, user should invoke
 * the yield function to begin computation work.
 * This function tells all awaiting workers to do the job and starts computation
 * itself in the thread where it was executed from, this thread is taken until
 * all computation tasks are computed, after this controls returns to the caller
 * and user can requeue tasks without actually creating new tasks from scratch
 * this is done by a specific scheduling algorithm. When a workers asks for
 * a task it gets it from a queue is there's any, but the task is not actually
 * removing from the queue, instead the task keep residing in the queue and a
 * pointer of the queue is only changing. So when user wants to change some
 * parameters in the computation algorithm, there's no need to rebuild a queue
 * over again, only the counter is going to be reset.
 *
 * Profiling.
 * The scheduler has an ability to record various performance counters and make
 * histograms from it. To enable this feature the scheduler must be built with a
 * CONFIG_RSCHED_PROFILE option.
 * Note, profiling may slightly decrease performance.
 * By default profiling enables all available profiling options and shows all
 * available counters and histograms, to disable and tune various profiling
 * options read the product documentation.
 *
 * Debugging.
 * For debugging the scheduler must be built with a CONFIG_RSCHED_DEBUG option.
 * Note, this can generate very massive verbose output.
 *
 */

#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>
#include <config/config.h>
#include <tools/compiler.h>

#include "rsched_queue.h"
#include "rsched_worker.h"
#include "rsched_common.h"



/* struct rsched - Main scheduler structure.
 *
 * @worker       - array of workers.
 * @n_workers    - count of workers.
 * @stats        - scheduler statistics including profile information.
 * @host_stats   - host worker statistics ( separated from worker structure ).
 * @user_fun     - A function for executing by workers.
 * @user_ctx     - A pointer to the user specific data, put to user_fun.
 * @queue        - Scheduler queue object.
 */
struct rsched
{
        struct rsched_worker* worker;
        uint32_t n_workers;

        struct rsched_stats stats;
        struct worker_stats host_stats;

        rsched_user_fun user_fun;
        void* user_ctx;

        __cache_aligned
        struct rsched_queue queue;
};


/* Create a scheduler with specified options */
int rsched_create(struct rsched** psched, struct rsched_options* opts);

/* Bind each thread to its own cpu/core */
int rsched_tune_thread_affinity(struct rsched* sched);


/* Split computing task represented as a rectangle with width and height
 * to sub-tasks NxM with specified grain.
 *
 * Keep in mind the greater grain size the less uniform CPU load
 * especially with non-linear algorithms.
 *
 * For example, on the machine with 1 CPU and 4 cores and given surface
 * of 1024x1024 size and grain size is 512x512 the surface will be split
 * into 4 blocks of size 512x512.
 *
 * Now imagine first block is easy for processing and takes 1500 ns to compute.
 * But last 3 blocks are hard and each takes up to 10 ms to compute.
 *
 * The scheduler will put all 4 tasks across all 4 cpu cores and when one core
 * will finish the first task, last 3 cores will be still busy with their tasks
 * but no other tasks left on the queue, so there's nothing left to do for
 * the free core except stalling without a useful work while other cores are
 * still busy, so CPU will be loaded approximately only on 75%.
 *
 * But if the surface was split into the blocks of 64x64 or less the situation
 * would be much better because when one worker ends with a task
 * it can immediately pull the next task from the queue
 * and keep doing useful work, so in average CPU load will be ~100%.
 *
 * Note, if the grain size is too small like 8x8 overhead of scheduling might
 * take significant amount of time.
 */
void rsched_create_tasks(struct rsched* sched, uint32_t width, uint32_t height,
                         struct block_size* grain);


/* Blocks current thread and gives control to the scheduler until all enqueued
 * tasks will be completed.
 */
int rsched_host_yield(struct rsched* sched);


/* Requeue earlier queued tasks without rebuilding the queue.
 * This function has absolutely no overhead.
 */
void rsched_requeue(struct rsched* sched);

/* Set user context */
void rsched_set_user_context(struct rsched* sched, rsched_user_fun fun,
                             void* user_ctx);


/* Shutdown and destroy scheduler and all workers */
void rsched_shutdown(struct rsched* sched);


/* Returns number of threads */
uint32_t rsched_threads_count(struct rsched* sched);
