#include <mdb/tools/atomic.h>
#include <malloc.h>
#include <string.h>
#include <mdb/tools/log.h>
#include "rsched_queue.h"
#include "rsched.h"


void rsched_queue_init(struct rsched_queue* queue)
{
        queue->tasks    = NULL;
        queue->capacity = 0;
        queue->length   = 0;
        atomic_store(&queue->cur_task_idx, 0);
}

void rsched_queue_destroy(struct rsched_queue* queue)
{
        free(queue->tasks);
        queue->tasks    = NULL;

        queue->length   = 0;
        queue->capacity = 0;

        atomic_store(&queue->cur_task_idx, 0);
}


void rsched_queue_resize(struct rsched_queue* queue,
                                uint32_t n, int flags)
{
        if(flags & RS_QUE_DISCARD
           && !(flags & RS_QUE_EXTEND))
        {
                free(queue->tasks);

                queue->tasks = calloc(n, sizeof(*queue->tasks));
                queue->capacity = n;
                queue->length = 0;

                atomic_store(&queue->cur_task_idx, 0);
        }
        else if(flags & RS_QUE_EXTEND
                && !(flags & RS_QUE_DISCARD))
        {
                uint32_t new_cap = queue->capacity + n;

                queue->tasks = realloc(queue->tasks,
                                       new_cap * sizeof(*queue->tasks));

                if(flags & RS_QUE_ZERO)
                {
                        memset(queue->tasks + queue->capacity,
                               0,
                               n * sizeof(*queue->tasks));
                }

                queue->capacity = new_cap;
        }
        else
        {
                LOG_ERROR("Unknown flags");
        }
}


void rsched_queue_push(struct rsched_queue* queue,
                              uint32_t x0, uint32_t x1,
                              uint32_t y0, uint32_t y1)
{
        struct rsched_task* t;

        if(queue->length >= queue->capacity)
        {
                uint32_t ext_n = queue->capacity / 4;

                LOG_WARN("Accessing out of range to the queue, "
                                 "{%i, %i, %i, %i}. Extending queue [%i]->[%i]",
                         x0, x1, y0, y1,
                         queue->capacity, queue->capacity + ext_n);

                rsched_queue_resize(queue,
                                    ext_n,
                                    RS_QUE_EXTEND
                                    | RS_QUE_ZERO);
        }

        t = &queue->tasks[queue->length++];
        t->x0 = x0;
        t->x1 = x1;
        t->y0 = y0;
        t->y1 = y1;
}

void rsched_split_task(struct rsched_queue* queue, uint32_t x0, uint32_t x1,
                       uint32_t y0, uint32_t y1, struct block_size* grain)
{
        uint32_t y, x, y11, x11;
        y = y0;

        while(y < y1)
        {
                y11 = MIN(y + grain->y, y1);

                x = x0;
                while(x < x1)
                {
                        x11 = MIN(x + grain->x, x1);
                        rsched_queue_push(queue, x, x11, y, y11);
                        x = x11;
                }
                y = y11;
        }
}
