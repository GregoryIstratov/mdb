#pragma once

#include <stdint.h>
#include <tools/atomic.h>

/*
 * Scheduler queue management
 */

enum
{
        /* Queue resize flags */

        /* Resize queue removing all old elements */
        RS_QUE_DISCARD = 1<<0,

        /* Resize queue append new elements to the old ones */
        RS_QUE_EXTEND  = 1<<1,

        /* Zero out new elements */
        RS_QUE_ZERO    = 1<<2
};

struct block_size
{
        uint32_t x, y;
};

struct rsched_task
{
        uint32_t x0, x1, y0, y1;
};

struct rsched_queue
{
        __atomic
        uint32_t cur_task_idx;

        struct rsched_task* tasks;

        uint32_t capacity;

        uint32_t length;
};

void rsched_queue_init(struct rsched_queue* queue);

void rsched_queue_destroy(struct rsched_queue* queue);

void rsched_queue_resize(struct rsched_queue* queue,
                         uint32_t n, int flags);

void rsched_queue_push(struct rsched_queue* queue,
                       uint32_t x0, uint32_t x1,
                       uint32_t y0, uint32_t y1);

static inline
struct rsched_task* rsched_queue_pop(struct rsched_queue* queue)
{
        uint32_t cur = atomic_load(&queue->cur_task_idx);

        if(cur >= queue->length)
                return NULL;


        while(!atomic_compare_exchange(&queue->cur_task_idx, &cur, cur + 1))
        {
                if(cur >= queue->length)
                        return NULL;

                ++cur;
        }

        return &queue->tasks[cur];
}

void rsched_split_task(struct rsched_queue* queue, uint32_t x0, uint32_t x1,
                       uint32_t y0, uint32_t y1, struct block_size* grain);


static inline
void rsched_queue_requeue(struct rsched_queue* queue)
{
        atomic_store(&queue->cur_task_idx, 0);
}
