#include "rsched.h"

#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>
#include <mdb/tools/utils.h>
#include <mdb/tools/atomic_x86.h>

#define PTHREAD_CHECK_RETURN(exp) { int ret; if((ret = (exp))) { fprintf(stderr, "%s: %s %s:%i\n", #exp, strerror(ret), __FILE__, __LINE__); exit(EXIT_FAILURE); } }
#define CHECK_RETURN_ERRNO(exp) { if(exp) { fprintf(stderr, "%s: %s %s:%i\n", #exp, strerror(errno), __FILE__, __LINE__); exit(EXIT_FAILURE); } }


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

void rsched_create(rsched** psched, uint32_t queue_len, uint32_t workers)
{
    *psched = (rsched*)calloc(1, sizeof(rsched));
    rsched* sched = *psched;

    sched->task_queue = (rsched_task*)calloc(queue_len, sizeof(rsched_task));

    atomic_store(&sched->cur_task, 0);

    sched->queue_top = 0;
    atomic_store(&sched->queue_bot, 0);

    PTHREAD_CHECK_RETURN(pthread_cond_init(&sched->cond, NULL))
    PTHREAD_CHECK_RETURN(pthread_mutex_init(&sched->mtx, NULL))


    sched->worker_threads = (pthread_t*)calloc(workers, sizeof(pthread_t));
    sched->worker_info = (struct rsched_worker_info*)calloc(workers, sizeof(struct rsched_worker_info));
    sched->n_workers = workers;
    sched->r_fun = NULL;
    sched->user_ctx = NULL;

    atomic_store(&sched->processed_tasks, 0);

    PTHREAD_CHECK_RETURN(pthread_mutex_lock(&sched->mtx))
    for(uint32_t i = 0; i < workers; ++i)
    {
        sched->worker_info[i].state = RC_WORKER_RUNNING;

        PTHREAD_CHECK_RETURN(pthread_create(&sched->worker_threads[i], NULL, &rsched_worker, sched))


        char name[32];
        snprintf(name, 32, "worker%u", i);
        PTHREAD_CHECK_RETURN(pthread_setname_np(sched->worker_threads[i], name))
    }

    while (!rsched_check_workers_state(sched, RC_WORKER_SLEEP))
    {
        pthread_cond_wait(&sched->cond, &sched->mtx);
    }

    PTHREAD_CHECK_RETURN(pthread_mutex_unlock(&sched->mtx))
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


void rsched_queue_resize(rsched* sched, uint32_t queue_len)
{
    free(sched->task_queue);

    sched->task_queue = (rsched_task*)calloc(queue_len, sizeof(rsched_task));

    atomic_store(&sched->cur_task, 0);

    sched->queue_top = 0;
}


void rsched_yield(rsched* sched, uint32_t caller)
{
    pthread_mutex_lock(&sched->mtx);

    if(caller == RENDER_SCHED_ROOT)
    {

#if defined(RSCHED_DEBUG_DETAIL)
        fprintf(stdout, "Main thread yield\n");
        fflush(stdout);
#endif

        if(!rsched_check_workers_state(sched, RC_WORKER_SLEEP))
        {
            fprintf(stderr, "[ERROR] Main thread entered sleep state before all workers were freed!\n");
            fflush(stdout);
        }

        rsched_set_workers_state(sched, RC_WORKER_PENDING_START);
        pthread_cond_broadcast(&sched->cond);

        while (!rsched_check_workers_state(sched, RC_WORKER_SLEEP))
        {
            pthread_cond_wait(&sched->cond, &sched->mtx);
        }

#if defined(RSCHED_DEBUG_DETAIL)
        fprintf(stdout, "Main thread waken up\n");
        fflush(stdout);
#endif

    }
    else if(caller == RENDER_SCHED_WORKER)
    {
        int this_worker = rsched_get_worker_id(sched, pthread_self());

#if defined(RSCHED_DEBUG_DETAIL)
        fprintf(stdout, "[%i] worker yields\n", this_worker);
        fflush(stdout);
#endif

        sched->worker_info[this_worker].state = RC_WORKER_SLEEP;

        if(rsched_check_workers_state(sched, RC_WORKER_SLEEP))
            pthread_cond_broadcast(&sched->cond);

        while (sched->worker_info[this_worker].state != RC_WORKER_PENDING_START)
        {
            pthread_cond_wait(&sched->cond, &sched->mtx);
        }

        sched->worker_info[this_worker].state = RC_WORKER_RUNNING;

#if defined(RSCHED_DEBUG_DETAIL)
        fprintf(stdout, "[%i] worker waken up\n", this_worker);
        fflush(stdout);
#endif

    }
    else
    {
        fprintf(stderr, "[rsched_yield] Unknown caller id: %u \n", caller);
        exit(EXIT_FAILURE);
    }

    pthread_mutex_unlock(&sched->mtx);
}

void rsched_push(rsched* sched, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1)
{
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

void rsched_split_task(rsched* sched, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1, uint32_t grain)
{
    uint32_t xsz = x1 - x0 + 1;
    uint32_t ysz = y1 - y0 + 1;

    if(xsz > grain)
    {
        uint32_t nxm = xsz / 2;
        uint32_t nx01 = x0 + (nxm - 1);
        uint32_t nx10 = x0 + nxm;

        rsched_split_task(sched, x0, nx01, y0, y1, grain);
        rsched_split_task(sched, nx10, x1, y0, y1, grain);
    }
    else if(ysz > grain)
    {
        uint32_t nym = ysz / 2;
        uint32_t ny01 = y0 + (nym - 1);
        uint32_t ny10 = y0 + nym;

        rsched_split_task(sched, x0, x1, y0, ny01, grain);
        rsched_split_task(sched, x0, x1, ny10, y1, grain);

    }
    else
    {
//        printf("block: %lux%lu\n", xsz, ysz);
//        printf("[%lu %lu]\n", x0, x1);
//        printf("[%lu %lu]\n", y0, y1);

        rsched_push(sched, x0, x1, y0, y1);
    }
}


static void* rsched_worker(void* arg)
{
    rsched* sched = (rsched*)arg;

    rsched_yield(sched, RENDER_SCHED_WORKER);

    for (;;)
    {
        rsched_task* t = rsched_pop(sched);

        if (t)
        {
            if(sched->r_fun == NULL)
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