#pragma once

#include <time.h>
#include <mdb/tools/log.h>
#include <mdb/tools/compiler.h>

#include "rsched_profile.h"

#if defined(NDEBUG) && defined(CONFIG_RSCHED_DEBUG)
#undef CONFIG_RSCHED_DEBUG
#endif

#define RSCHED_DEBUG(fmt, ...) if(IS_ENABLED(CONFIG_RSCHED_DEBUG)) \
                                     LOG_DEBUG(fmt, ##__VA_ARGS__)


typedef void(* rsched_user_fun)(uint32_t x0, uint32_t x1,
                                uint32_t y0, uint32_t y1,
                                void* ctx);

struct rsched_options
{
        uint32_t threads;

        struct rsched_profile_options profile;
};

static inline
void rsched_yield_cpu(void)
{
        sched_yield();
}
