#pragma once

#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>

//#define RSCHED_DEBUG_DETAIL

enum
{
    RENDER_SCHED_ROOT = 0,
    RENDER_SCHED_WORKER = 1
};


typedef void(* rsched_proc_fun)(uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1, void* ctx);

typedef struct _rsched rsched;

void rsched_create(rsched** psched, uint32_t queue_len, uint32_t workers);

void rsched_set_proc_fun(rsched* sched, rsched_proc_fun fun, void* user_ctx);

void rsched_split_task(rsched* sched, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1, uint32_t grain);

void rsched_shutdown(rsched* sched);

uint32_t rsched_get_workers_count(rsched* sched);

void rsched_requeue(rsched* sched);

void rsched_queue_resize(rsched* sched, uint32_t queue_len);

void rsched_yield(rsched* sched, uint32_t caller);

void rsched_push(rsched* sched, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);

//rsched_task* rsched_pop(rsched* sched);

uint32_t rsched_enqueued_tasks(rsched* sched);