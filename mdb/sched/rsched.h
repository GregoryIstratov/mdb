#pragma once

#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct _rsched rsched;

typedef void(* rsched_proc_fun)(uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1, void* ctx);

struct block_size
{
    uint32_t x, y;
};

enum
{
    RSCHED_ROOT = 0,
    RSCHED_WORKER = 1
};

/* Create scheduler with n workers
 * This scheduler uses static length common queue for tasks
 * that creates and fills at preparation stage and left immutable
 * during processing.
 * This allows use compact very fast algorithms with very small overhead
 * to retrieve tasks from the queue and use one queue for all threads
 * it gives a very good load distribution across CPU cores.
 * */
void rsched_create(rsched** psched, uint32_t workers);

/* Split computing task represented as rectangle with width and height
 * to subtasks NxM with specified grain.
 * Keep in mind the greater grain size the less uniform cpu load
 * especially with the mandelbrot fractal where some pieces of surface
 * may require more than 256 and more ( depends on bailout ) iterations
 * to compute pixel than other pieces.
 * This is why if you for example have 4 cpu cores, surface 1024x1024 and grain
 * size 512x512 surface will be split into 4 blocks of 512x512 size
 * where one block will be super easy for computation but others will be hard
 * and scheduler will put all 4 tasks across all 4 cpu cores
 * and when one core with easy block will finish computation
 * there's nothing left to do for that core except stalling without useful work
 * while other cores still fighting with their hard tasks.
 * But if surface were split into blocks of 64x64 or even 128x128
 * the situation would be much much better, because when one worker ends with easy task
 * its can immediately pull the next task from the queue and keep doing useful work
 * in this case hard and easy pixels will be better mixed in the scheduler queue
 * and cores won't stall without useful work.
 * But there's another side of the coin, if block size is too small
 * there will be much overhead of pulling tasks from the queue, setting needed parameters
 * in the kernel and bad CPU cache coherency.
 * So choosing the right grain size is very important thing and depends on many factors
 * like number of CPU cores, core speed, cache size, surface size, bailout, etc.
 * But there's a few tested good grain sizes which work good with many regular
 * parameters - try 64x64, 128x64, 128x128 and 256x256 if you have a very big surface.
 * */
void rsched_create_tasks(rsched* sched, uint32_t width, uint32_t height, struct block_size* grain);

/* Gives control to either workers or root thread.
 * If caller RSCHED_ROOT blocks current thread and gives control to workers
 * until all enqueued tasks will be computed, then if there's nothing left on the
 * queue, workers call this function with caller RSCHED_WORKER and switches to sleep
 * state, when all workers yield, the thread earlier called this function wakes
 * and receives control back.
 * This function must be called from outside only with RSCHED_ROOT caller argument.
 */
void rsched_yield(rsched* sched, uint32_t caller);


/* Requeue earlier queued tasks without recomputing anything
 * and rebuilding the queue.
 * This is needed when you do the same work in the loop for
 * measuring performance or real-time rendering to screen.
 * Usually this function calls after receiving control after
 * rsched_yield then you can yield again with a new queue.
 * This function absolutely has no overhead.
 */
void rsched_requeue(rsched* sched);

/* Set processing function callback */
void rsched_set_proc_fun(rsched* sched, rsched_proc_fun fun, void* user_ctx);

/* Shutdown and destroy scheduler and all workers by calling cancel function */
void rsched_shutdown(rsched* sched);

/* Returns number of workers */
uint32_t rsched_get_workers_count(rsched* sched);

/* Returns number of tasks in the scheduler queue */
uint32_t rsched_enqueued_tasks(rsched* sched);