#pragma once

#include <time.h>
#include <tools/log.h>
#include <tools/compiler.h>

#include "rsched_profile.h"

#if defined(CONFIG_RSCHED_DEBUG)
#error Rsched debug system has moved to a separate patch file, you must apply \
it before enabling
#endif

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
