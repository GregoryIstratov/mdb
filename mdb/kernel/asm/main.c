
int run_kernel();

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <syscall.h>
#include <unistd.h>

#include <time.h>
#include <locale.h>

#include "mdb_asm_kernel.h"

#include <pthread.h>
#include <stdatomic.h>
#include <sched.h>
#include <linux/sched.h>

#include "sched/rsched.h"
#include "ogl_render.h"


#define PRINT_ERR(msg,...) { fprintf(stderr, msg, ##__VA_ARGS__); fprintf(stderr, ": %m %s:%i\n", __FILE__, __LINE__); }
#define PRINT_ERR_EXIT(msg, ... ) { PRINT_ERR(msg, ##__VA_ARGS__); exit(EXIT_FAILURE); }
#define LOG_PARAM(label, fmt, ...) { fprintf(stdout, "%-20s: ", (label)); fprintf(stdout, (fmt), ##__VA_ARGS__); fprintf(stdout,"\n"); fflush(stdout); }
#define LOG_DEBUG(label, fmt, ...) LOG_PARAM(label, fmt, ##__VA_ARGS__)
#define PTHREAD_CHECK_RETURN(exp) { int ret; if((ret = (exp))) { fprintf(stderr, "%s: %s %s:%i\n", #exp, strerror(ret), __FILE__, __LINE__); exit(EXIT_FAILURE); } }
#define CHECK_RETURN_ERRNO(exp) { if(exp) { fprintf(stderr, "%s: %s %s:%i\n", #exp, strerror(errno), __FILE__, __LINE__); exit(EXIT_FAILURE); } }

struct raw_header
{
    unsigned long width, height;
};

void save_surface(const void* data, unsigned long width, unsigned long height)
{

    struct raw_header header;
    header.width  = width;
    header.height = height;

    FILE* f = fopen("mandelbrot.raw", "wb");

    if(!f)
    {
        PRINT_ERR_EXIT("[save_surface]: Can't open the file")
    }

    size_t size = width * height * 4;

    fwrite(&header, sizeof(header), 1, f);

    size_t res = fwrite(data, 1, size, f);

    if(res != size)
    {
        PRINT_ERR_EXIT("[save_surface]: fwrite failed");
    }


    fclose(f);
}


static double get_total_sec(const struct timespec* ts)
{
    static const double NS_IN_SEC = 1000000000;
    if (ts->tv_sec == 0)
        return (double) ts->tv_nsec / NS_IN_SEC;

    double total_sec;
    total_sec = (double) ts->tv_nsec / NS_IN_SEC;
    total_sec += (double) ts->tv_sec;

    return total_sec;
}


static double sample_timer(void)
{
    struct timespec tm;

    clock_gettime(CLOCK_MONOTONIC, &tm);

    return get_total_sec(&tm);

}

static void print_statistics(uint64_t width, uint64_t height, uint64_t frames, double elapsed)
{
    LOG_PARAM("Total seconds", "%f", elapsed);
    LOG_PARAM("Pixels/Second", "%'lu", (uint64_t)((double)(width*height)/elapsed));
    LOG_PARAM("Total frames","%lu", frames);
    LOG_PARAM("Frames/Second","%f", ((double)frames/elapsed));
}

void print_clocks(uint64_t begin, uint64_t end)
{
    uint64_t elapsed = end - begin;
    printf("Total cycles: %'lu\n", elapsed);
    printf("Cycles/Pixel: %f\n", ((double)elapsed/(1024*1024)));
}


typedef struct
{
    uint32_t x0,x1,y0,y1;
} render_sched_task;

enum
{
    RENDER_SCHED_ROOT   = 0,
    RENDER_SCHED_WORKER = 1
};


static void* render_sched_worker(void* arg);

enum
{
    RC_WORKER_RUNNING,
    RC_WORKER_SLEEP,
    RC_WORKER_PENDING_START,
};

struct rc_worker_info
{
    int state;
};

typedef struct
{
    render_sched_task*           task_queue;
    atomic_int                   cur_task;
    uint32_t                     queue_top;
    atomic_int                   queue_bot;

    pthread_mutex_t              mtx;
    pthread_cond_t               cond;

    pthread_t*                   worker_threads;
    struct rc_worker_info*       worker_info;
    uint32_t                     n_workers;

    atomic_int                   processed_tasks;
} render_sched;

static int render_sched_check_workers_state(render_sched* sched, int state);

static void render_sched_create(render_sched* sched, uint32_t queue_len, uint32_t workers)
{
    sched->task_queue = (render_sched_task*)calloc(queue_len, sizeof(render_sched_task));

    atomic_store_explicit(&sched->cur_task, 0, memory_order_release);

    sched->queue_top = 0;
    atomic_store_explicit(&sched->queue_bot, 0, memory_order_release);

    PTHREAD_CHECK_RETURN(pthread_cond_init(&sched->cond, NULL))
    PTHREAD_CHECK_RETURN(pthread_mutex_init(&sched->mtx, NULL))


    sched->worker_threads = (pthread_t*)calloc(workers, sizeof(pthread_t));
    sched->worker_info = (struct rc_worker_info*)calloc(workers, sizeof(struct rc_worker_info));
    sched->n_workers = workers;

    atomic_store_explicit(&sched->processed_tasks, 0, memory_order_release);

    PTHREAD_CHECK_RETURN(pthread_mutex_lock(&sched->mtx))
    for(uint32_t i = 0; i < workers; ++i)
    {
        sched->worker_info[i].state = RC_WORKER_RUNNING;

        PTHREAD_CHECK_RETURN(pthread_create(&sched->worker_threads[i], NULL, &render_sched_worker, sched))


        char name[32];
        snprintf(name, 32, "worker%u", i);
        PTHREAD_CHECK_RETURN(pthread_setname_np(sched->worker_threads[i], name))
    }

    while (render_sched_check_workers_state(sched, RC_WORKER_SLEEP) != 0)
    {
        pthread_cond_wait(&sched->cond, &sched->mtx);
    }

    PTHREAD_CHECK_RETURN(pthread_mutex_unlock(&sched->mtx))
}

static void render_sched_shutdown(render_sched* sched)
{
    for(uint32_t i = 0; i < sched->n_workers; ++i)
    {
        pthread_cancel(sched->worker_threads[i]);
    }

}

static int render_sched_get_worker_id(render_sched* sched, pthread_t p_tid)
{
    for(uint32_t i = 0; i < sched->n_workers; ++i)
    {
        if(sched->worker_threads[i] == p_tid)
            return i;
    }

    return -1;

}

static int render_sched_check_workers_state(render_sched* sched, int state)
{
    for(uint32_t i = 0; i < sched->n_workers; ++i)
    {
        if(sched->worker_info[i].state != state)
            return -1;
    }

    return 0;
}

static int render_sched_set_workers_state(render_sched* sched, int state)
{
    for(uint32_t i = 0; i < sched->n_workers; ++i)
    {
        sched->worker_info[i].state = state;
    }

    return 0;
}


static void render_sched_requeue(render_sched* sched)
{
    atomic_store_explicit(&sched->cur_task, 0, memory_order_release);
}


static void render_sched_queue_resize(render_sched* sched, uint32_t queue_len)
{
    free(sched->task_queue);

    sched->task_queue = (render_sched_task*)calloc(queue_len, sizeof(render_sched_task));

    atomic_store_explicit(&sched->cur_task, 0, memory_order_release);

    sched->queue_top = 0;
}


static void render_sched_yield(render_sched* sched, uint32_t caller)
{
    pthread_mutex_lock(&sched->mtx);

    if(caller == RENDER_SCHED_ROOT)
    {
        fprintf(stdout, "Main thread yield\n");
        fflush(stdout);

        if(render_sched_check_workers_state(sched, RC_WORKER_SLEEP) != 0)
        {
            fprintf(stderr, "[ERROR] Main thread entered sleep state before all workers were freed!\n");
            fflush(stdout);
        }

        render_sched_set_workers_state(sched, RC_WORKER_PENDING_START);
        pthread_cond_broadcast(&sched->cond);

        while (render_sched_check_workers_state(sched, RC_WORKER_SLEEP) != 0)
        {
            pthread_cond_wait(&sched->cond, &sched->mtx);
        }

        fprintf(stdout, "Main thread waken up\n");
        fflush(stdout);

    }
    else if(caller == RENDER_SCHED_WORKER)
    {
        int this_worker = render_sched_get_worker_id(sched, pthread_self());

        fprintf(stdout, "[%i] worker yields\n", this_worker);
        fflush(stdout);


        sched->worker_info[this_worker].state = RC_WORKER_SLEEP;

        if(render_sched_check_workers_state(sched, RC_WORKER_SLEEP) == 0)
            pthread_cond_broadcast(&sched->cond);

        while (sched->worker_info[this_worker].state != RC_WORKER_PENDING_START)
        {
            pthread_cond_wait(&sched->cond, &sched->mtx);
        }

        sched->worker_info[this_worker].state = RC_WORKER_RUNNING;

        fprintf(stdout, "[%i] worker waken up\n", this_worker);
        fflush(stdout);
    }
    else
    {
        fprintf(stderr, "[render_sched_yield] Unknown caller id: %u \n", caller);
        exit(EXIT_FAILURE);
    }

    pthread_mutex_unlock(&sched->mtx);
}

static void render_sched_push(render_sched* sched, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1)
{
    render_sched_task* t = &sched->task_queue[sched->queue_top++];
    t->x0 = x0;
    t->x1 = x1;
    t->y0 = y0;
    t->y1 = y1;
}


static render_sched_task* render_sched_pop(render_sched* sched)
{
    int cur = atomic_load_explicit(&sched->cur_task, memory_order_acquire);

    if(cur == sched->queue_top)
        return NULL;


    while(!atomic_compare_exchange_weak_explicit(&sched->cur_task, &cur, cur+1, memory_order_acq_rel, memory_order_acquire))
    {
        if(cur == sched->queue_top)
            return NULL;

        ++cur;
    }

    return &sched->task_queue[cur];
}

static uint32_t render_sched_enqueued_tasks(render_sched* sched)
{
    return sched->queue_top;
}

static void render_sched_print_summary(render_sched* sched)
{
    uint32_t processed_tasks = atomic_load_explicit(&sched->processed_tasks, memory_order_acquire);

    LOG_DEBUG("Processed tasks", "%u", processed_tasks);
}


static void* render_sched_worker(void* arg)
{
    render_sched* sched = (render_sched*)arg;

    render_sched_yield(sched, RENDER_SCHED_WORKER);

    for (;;)
    {
        render_sched_task* t = render_sched_pop(sched);

        if (t)
        {
            mdbt_kernel(t->x0, t->x1, t->y0, t->y1);
            atomic_fetch_add_explicit(&sched->processed_tasks, 1, memory_order_acq_rel);
        }
        else
        {
            render_sched_yield(sched, RENDER_SCHED_WORKER);
        }
    }

    return NULL;
}

static void dac_split_task(render_sched* sched, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1, uint32_t grain)
{
    uint32_t xsz = x1 - x0 + 1;
    uint32_t ysz = y1 - y0 + 1;

    if(xsz > grain)
    {
        uint32_t nxm = xsz / 2;
        uint32_t nx01 = x0 + (nxm - 1);
        uint32_t nx10 = x0 + nxm;

        dac_split_task(sched, x0, nx01, y0, y1, grain);
        dac_split_task(sched, nx10, x1, y0, y1, grain);
    }
    else if(ysz > grain)
    {
        uint32_t nym = ysz / 2;
        uint32_t ny01 = y0 + (nym - 1);
        uint32_t ny10 = y0 + nym;

        dac_split_task(sched, x0, x1, y0, ny01, grain);
        dac_split_task(sched, x0, x1, ny10, y1, grain);

    }
    else
    {
//        printf("block: %lux%lu\n", xsz, ysz);
//        printf("[%lu %lu]\n", x0, x1);
//        printf("[%lu %lu]\n", y0, y1);

        render_sched_push(sched, x0, x1, y0, y1);
    }
}

static void mdbt_test_norender()
{
    uint32_t width     = 1024;
    uint32_t height    = width;
    uint32_t frames    = 4000;
    uint32_t grain     = 64;

    struct sched_param param;
    param.sched_priority = 0;

    CHECK_RETURN_ERRNO(sched_setscheduler(0, SCHED_BATCH, &param))

    setlocale(LC_ALL, "");

    uint64_t buff_size = width * height * 4;
    //float* surface = (float*)calloc(1, buff_size);
    float* surface;
    posix_memalign((void**)&surface, 32, buff_size);
    memset(surface, 0, buff_size);

    mdbt_set_surface(surface);
    mdbt_set_scale(0.00188964f);
    mdbt_set_shift(-1.347385054652062f, 0.063483549665202f);

    //mdbt_set_scale(2.5f);
    //mdbt_set_shift(-0.7f, 0.0f);
    mdbt_set_size(width, height);
    mdbt_set_bailout(256);
    mdbt_compute_transpose_offset();

    render_sched sched;
    render_sched_create(&sched, width * height / grain, 4);

    dac_split_task(&sched, 0, width-1, 0, height-1, grain);

    LOG_DEBUG("Enqueued tasks", "%u", render_sched_enqueued_tasks(&sched));
    LOG_DEBUG("Expected to process", "%u", render_sched_enqueued_tasks(&sched)*frames);

    double start = sample_timer();
    for(uint32_t i = 0; i < frames; ++i)
    {
        render_sched_yield(&sched, RENDER_SCHED_ROOT);

        render_sched_requeue(&sched);
    }

    double end = sample_timer();

    render_sched_print_summary(&sched);

    render_sched_shutdown(&sched);

    double elapsed = end - start;

    print_statistics(width, height, frames, elapsed);

    save_surface(surface, width, height);

    free(surface);
}

struct mdbt_ogl_render_ctx
{
    render_sched* sched;
    float shift_x, shift_y, scale;
    uint32_t bailout;
    uint32_t width;
    uint32_t height;
    uint32_t grain;
};

#include <GLFW/glfw3.h>

static void mdbt_render_update_scale(struct mdbt_ogl_render_ctx* ctx)
{
    mdbt_set_scale(ctx->scale);
    mdbt_compute_transpose_offset();
    LOG_PARAM("scale", "%f", ctx->scale);
}

static void mdbt_render_update_shift(struct mdbt_ogl_render_ctx* ctx)
{
    mdbt_set_shift(ctx->shift_x, ctx->shift_y);
    mdbt_compute_transpose_offset();
    LOG_PARAM("shift", "%f %f", ctx->shift_x, ctx->shift_y);
}

static void mdbt_render_update_bailout(struct mdbt_ogl_render_ctx* ctx)
{
    mdbt_set_bailout(ctx->bailout);
    LOG_PARAM("bailout", "%u", ctx->bailout);
}

static uint32_t iterations_get_mod(struct mdbt_ogl_render_ctx* ctx)
{
    if (ctx->bailout < 256)
        return 1;
    if (ctx->bailout < 512)
        return 2;
    if (ctx->bailout < 1024)
        return 4;
    if (ctx->bailout < 2048)
        return 8;
    if (ctx->bailout < 4096)
        return 32;
    if (ctx->bailout < 8192)
        return 128;
    if (ctx->bailout < 16384)
        return 256;

    return 512;
}

static void iterations_increase(struct mdbt_ogl_render_ctx* ctx)
{
    ctx->bailout += iterations_get_mod(ctx);
}

static void iterations_decrease(struct mdbt_ogl_render_ctx* ctx)
{
    if (ctx->bailout > 1) ctx->bailout -= iterations_get_mod(ctx);
}

static void shift_right(struct mdbt_ogl_render_ctx* ctx)
{
    ctx->shift_x += 0.1 * ctx->scale;
}

static void shift_left(struct mdbt_ogl_render_ctx* ctx)
{
    ctx->shift_x -= 0.1 * ctx->scale;
}

static void shift_up(struct mdbt_ogl_render_ctx* ctx)
{
    ctx->shift_y -= 0.1 * ctx->scale;
}

static void shift_down(struct mdbt_ogl_render_ctx* ctx)
{
    ctx->shift_y += 0.1 * ctx->scale;
}

static void mdbt_render_key_callback(int key, void* user_ctx)
{
    struct mdbt_ogl_render_ctx* ctx = (struct mdbt_ogl_render_ctx*)user_ctx;

    switch (key)
    {
        case GLFW_KEY_1:
        {
            ctx->scale *= 1.1;
            mdbt_render_update_scale(ctx);
            return;
        }
        case GLFW_KEY_2:
        {
            ctx->scale *= 0.9;
            mdbt_render_update_scale(ctx);
            return;
        }
        case GLFW_KEY_3:
        {
            iterations_decrease(ctx);
            mdbt_render_update_bailout(ctx);
            return;
        }
        case GLFW_KEY_4:
        {
            iterations_increase(ctx);
            mdbt_render_update_bailout(ctx);
            return;
        }
        case GLFW_KEY_RIGHT:
        {
            shift_right(ctx);
            mdbt_render_update_shift(ctx);
            return;
        }
        case GLFW_KEY_LEFT:
        {
            shift_left(ctx);
            mdbt_render_update_shift(ctx);
            return;
        }
        case GLFW_KEY_UP:
        {
            shift_up(ctx);
            mdbt_render_update_shift(ctx);
            return;
        }
        case GLFW_KEY_DOWN:
        {
            shift_down(ctx);
            mdbt_render_update_shift(ctx);
            return;
        }
    }
}

static void mdbt_render_resize(int width, int height, void* context)
{
    struct mdbt_ogl_render_ctx* ctx = (struct mdbt_ogl_render_ctx*)context;

    ctx->width  = (uint32_t)width;
    ctx->height = (uint32_t)height;

    mdbt_set_size(ctx->width, ctx->height);
    mdbt_compute_transpose_offset();

    render_sched_queue_resize(ctx->sched, ctx->width * ctx->height / ctx->grain);

    dac_split_task(ctx->sched, 0, ctx->width-1, 0, ctx->height-1, ctx->grain);

}

static void mdbt_render_update(void* data, void* context)
{
    struct mdbt_ogl_render_ctx* ctx = (struct mdbt_ogl_render_ctx*)context;

    mdbt_set_surface((float*)data);

    render_sched_yield(ctx->sched, RENDER_SCHED_ROOT);

    render_sched_requeue(ctx->sched);
}

static void mdbt_test_olg_render()
{
    uint32_t width     = 1024;
    uint32_t height    = width;
    uint32_t grain     = 64;
    struct mdbt_ogl_render_ctx ctx;

    ctx.bailout = 256;
    ctx.scale   = 0.00188964f;
    ctx.shift_x = -1.347385054652062f;
    ctx.shift_y = 0.063483549665202f;
    ctx.width   = width;
    ctx.height  = height;
    ctx.grain   = grain;

    struct sched_param param;
    param.sched_priority = 0;

    CHECK_RETURN_ERRNO(sched_setscheduler(0, SCHED_BATCH, &param))

    setlocale(LC_ALL, "");

    mdbt_set_scale(ctx.scale);
    mdbt_set_shift(ctx.shift_x, ctx.shift_y);

    //mdbt_set_scale(2.5f);
    //mdbt_set_shift(-0.7f, 0.0f);
    mdbt_set_size(width, height);
    mdbt_set_bailout(ctx.bailout);
    mdbt_compute_transpose_offset();

    render_sched sched;
    render_sched_create(&sched, width * height / grain, 4);

    dac_split_task(&sched, 0, width-1, 0, height-1, grain);

    LOG_DEBUG("Enqueued tasks", "%u", render_sched_enqueued_tasks(&sched));


    ctx.sched = &sched;

    ogl_render* rend;
    ogl_render_create(&rend, width, height, &mdbt_render_update, &ctx, &mdbt_render_key_callback);
    ogl_render_set_resize_callback(rend, &mdbt_render_resize);


    ogl_render_render_loop(rend);

    render_sched_print_summary(&sched);

    render_sched_shutdown(&sched);

    ogl_render_destroy(rend);
}

int main(void)
{
    mdbt_test_olg_render();
    return 0;
}