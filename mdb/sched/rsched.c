#include "rsched.h"

#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>
#include <mdb/tools/utils.h>
#include <mdb/tools/atomic_x86.h>
#include <mdb/tools/log.h>

enum
{
    RC_WORKER_RUNNING,
    RC_WORKER_SLEEP,
    RC_WORKER_PENDING_START,
};

typedef struct
{
    uint32_t x0, x1, y0, y1;
} rsched_task;

struct rsched_worker_info
{
    int state;
};

struct _rsched
{
    rsched_task* task_queue;
    uint32_t     queue_len;

    __atomic uint32_t cur_task;
    uint32_t queue_top;
    __atomic uint32_t queue_bot;

    pthread_mutex_t mtx;
    pthread_cond_t cond;

    rsched_proc_fun r_fun;
    void* user_ctx;

    pthread_t* worker_threads;
    struct rsched_worker_info* worker_info;
    uint32_t n_workers;

    __atomic int32_t processed_tasks;
};

static void* rsched_worker(void* arg);
static bool rsched_check_workers_state(rsched* sched, int state);
static void rsched_set_workers_state(rsched* sched, int state);

void rsched_create(rsched** psched, uint32_t workers)
{
    *psched = (rsched*)calloc(1, sizeof(rsched));
    rsched* sched = *psched;

    atomic_store(&sched->cur_task, 0);

    sched->queue_top = 0;
    atomic_store(&sched->queue_bot, 0);

    int ret = 0;
    if((ret = pthread_cond_init(&sched->cond, NULL)))
    {
        LOG_ERROR("[pthread_cond_init]: %s", strerror(ret));
        exit(EXIT_FAILURE);
    }


    if((ret = pthread_mutex_init(&sched->mtx, NULL)))
    {
        LOG_ERROR("[pthread_mutex_init]: %s", strerror(ret));
        exit(EXIT_FAILURE);
    }


    sched->worker_threads = (pthread_t*)calloc(workers, sizeof(pthread_t));
    sched->worker_info = (struct rsched_worker_info*)calloc(workers, sizeof(struct rsched_worker_info));
    sched->n_workers = workers;
    sched->r_fun = NULL;
    sched->user_ctx = NULL;

    atomic_store(&sched->processed_tasks, 0);

    pthread_mutex_lock(&sched->mtx);
    for(uint32_t i = 0; i < workers; ++i)
    {
        sched->worker_info[i].state = RC_WORKER_RUNNING;

        if((ret = pthread_create(&sched->worker_threads[i], NULL, &rsched_worker, sched)))
        {
            LOG_ERROR("[pthread_create]: %s", strerror(ret));
            exit(EXIT_FAILURE);
        }


        char name[32];
        snprintf(name, 32, "worker%u", i);

        if((ret = pthread_setname_np(sched->worker_threads[i], name)))
        {
            LOG_ERROR("[pthread_setname_np]: %s", strerror(ret));
            exit(EXIT_FAILURE);
        }
    }

    while (!rsched_check_workers_state(sched, RC_WORKER_SLEEP))
    {
        pthread_cond_wait(&sched->cond, &sched->mtx);
    }

    pthread_mutex_unlock(&sched->mtx);
}

void rsched_set_proc_fun(rsched* sched, rsched_proc_fun fun, void* user_ctx)
{
    sched->r_fun = fun;
    sched->user_ctx = user_ctx;
}

void rsched_shutdown(rsched* sched)
{
    for(uint32_t i = 0; i < sched->n_workers; ++i)
    {
        pthread_cancel(sched->worker_threads[i]);
    }

}

static int rsched_get_worker_id(rsched* sched, pthread_t p_tid)
{
    for(uint32_t i = 0; i < sched->n_workers; ++i)
    {
        if(sched->worker_threads[i] == p_tid)
            return i;
    }

    return -1;

}

uint32_t rsched_get_workers_count(rsched* sched)
{
    return sched->n_workers;
}

static bool rsched_check_workers_state(rsched* sched, int state)
{
    for(uint32_t i = 0; i < sched->n_workers; ++i)
    {
        if(sched->worker_info[i].state != state)
            return false;
    }

    return true;
}

static void rsched_set_workers_state(rsched* sched, int state)
{
    for(uint32_t i = 0; i < sched->n_workers; ++i)
    {
        sched->worker_info[i].state = state;
    }
}


void rsched_requeue(rsched* sched)
{
    atomic_store(&sched->cur_task, 0);
}

enum
{
    RSCHED_QUEUE_RESIZE_DISCARD = 1,
    RSCHED_QUEUE_RESIZE_EXTEND  = 1<<1,
    RSCHED_QUEUE_RESIZE_ZERO    = 1<<2
};

static void rsched_queue_resize(rsched* sched, uint32_t queue_len, int flags)
{
    if(flags & RSCHED_QUEUE_RESIZE_DISCARD && !(flags & RSCHED_QUEUE_RESIZE_EXTEND))
    {
        free(sched->task_queue);

        sched->task_queue = (rsched_task*) calloc(queue_len, sizeof(rsched_task));
        sched->queue_len = queue_len;

        atomic_store(&sched->cur_task, 0);

        sched->queue_top = 0;
    }
    else if(flags & RSCHED_QUEUE_RESIZE_EXTEND && !(flags & RSCHED_QUEUE_RESIZE_DISCARD))
    {
        uint32_t qlen = sched->queue_len + queue_len;
        sched->task_queue = (rsched_task*) realloc(sched->task_queue, qlen * sizeof(rsched_task));

        if(flags & RSCHED_QUEUE_RESIZE_ZERO)
        {
            memset(sched->task_queue+sched->queue_len, 0, queue_len * sizeof(rsched_task));
        }

        sched->queue_len = qlen;

    }
    else
    {
        LOG_ERROR("Unknown flags");
    }
}


void rsched_yield(rsched* sched, uint32_t caller)
{
    pthread_mutex_lock(&sched->mtx);

    if(caller == RENDER_SCHED_ROOT)
    {
        if(!rsched_check_workers_state(sched, RC_WORKER_SLEEP))
        {
            LOG_ERROR("[rsched_yield] Serious bug discovered! "
                              "Main thread entered to the sleep state before all workers were freed! "
                              "This can cause a fatal error or hang the program!");
        }

        rsched_set_workers_state(sched, RC_WORKER_PENDING_START);
        pthread_cond_broadcast(&sched->cond);

        while (!rsched_check_workers_state(sched, RC_WORKER_SLEEP))
        {
            pthread_cond_wait(&sched->cond, &sched->mtx);
        }

    }
    else if(caller == RENDER_SCHED_WORKER)
    {
        int this_worker = rsched_get_worker_id(sched, pthread_self());

        sched->worker_info[this_worker].state = RC_WORKER_SLEEP;

        if(rsched_check_workers_state(sched, RC_WORKER_SLEEP))
            pthread_cond_broadcast(&sched->cond);

        while (sched->worker_info[this_worker].state != RC_WORKER_PENDING_START)
        {
            pthread_cond_wait(&sched->cond, &sched->mtx);
        }

        sched->worker_info[this_worker].state = RC_WORKER_RUNNING;
    }
    else
    {
        LOG_ERROR("Unknown caller id: %u", caller);
        exit(EXIT_FAILURE);
    }

    pthread_mutex_unlock(&sched->mtx);
}

void rsched_push(rsched* sched, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1)
{
    if(sched->queue_top >= sched->queue_len)
    {
        uint32_t ext_len = sched->queue_len / 4;
        LOG_WARN("Detected attempt of out of range accessing to the queue, element {%i, %i, %i, %i}. Extending queue [%i]->[%i]\n",
                x0, x1, y0, y1,
                sched->queue_len, sched->queue_len + ext_len);

        rsched_queue_resize(sched, ext_len, RSCHED_QUEUE_RESIZE_EXTEND | RSCHED_QUEUE_RESIZE_ZERO);
    }

    rsched_task* t = &sched->task_queue[sched->queue_top++];
    t->x0 = x0;
    t->x1 = x1;
    t->y0 = y0;
    t->y1 = y1;
}


static rsched_task* rsched_pop(rsched* sched)
{
    uint32_t cur = atomic_load(&sched->cur_task);

    if(cur >= sched->queue_top)
        return NULL;


    while(!atomic_compare_exchange(&sched->cur_task, &cur, cur+1))
    {
        if(cur >= sched->queue_top)
            return NULL;

        ++cur;
    }

    return &sched->task_queue[cur];
}

uint32_t rsched_enqueued_tasks(rsched* sched)
{
    return sched->queue_top;
}

//static void rsched_print_summary(rsched* sched)
//{
//    uint32_t processed_tasks = atomic_load_explicit(&sched->processed_tasks, memory_order_acquire);
//
//    LOG_DEBUG("Processed tasks", "%u", processed_tasks);
//}

static void rsched_split_task(rsched* sched, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1, struct block_size* grain)
{
    uint32_t xsz = x1 - x0 + 1;
    uint32_t ysz = y1 - y0 + 1;

    if(xsz > grain->x)
    {
        uint32_t nxm = xsz / 2;
        uint32_t nx01 = x0 + (nxm - 1);
        uint32_t nx10 = x0 + nxm;

        rsched_split_task(sched, x0, nx01, y0, y1, grain);
        rsched_split_task(sched, nx10, x1, y0, y1, grain);
    }
    else if(ysz > grain->y)
    {
        uint32_t nym = ysz / 2;
        uint32_t ny01 = y0 + (nym - 1);
        uint32_t ny10 = y0 + nym;

        rsched_split_task(sched, x0, x1, y0, ny01, grain);
        rsched_split_task(sched, x0, x1, ny10, y1, grain);

    }
    else
    {
        rsched_push(sched, x0, x1, y0, y1);
    }
}

void rsched_create_tasks(rsched* sched, uint32_t width, uint32_t height, struct block_size* grain)
{
    uint32_t wxh = width * height;
    uint32_t grain2 = grain->x * grain->y;
    uint32_t qlen = wxh / grain2 + (wxh % grain2 != 0);

    rsched_queue_resize(sched, qlen, RSCHED_QUEUE_RESIZE_DISCARD | RSCHED_QUEUE_RESIZE_ZERO);

    rsched_split_task(sched, 0, width-1, 0, height-1, grain);
}


static void* rsched_worker(void* arg)
{
    rsched* sched = (rsched*)arg;

    rsched_yield(sched, RENDER_SCHED_WORKER);

    for (;;)
    {
        rsched_task* t = rsched_pop(sched);

        if (likely(t))
        {
            if(unlikely(sched->r_fun == NULL))
            {
                LOG_ERROR("[rsched_worker] processor function is not set\n");
                exit(EXIT_FAILURE);
            }

            sched->r_fun(t->x0, t->x1, t->y0, t->y1, sched->user_ctx);
            atomic_fetch_add(&sched->processed_tasks, 1);
        }
        else
        {
            rsched_yield(sched, RENDER_SCHED_WORKER);
        }
    }

    return NULL;
}