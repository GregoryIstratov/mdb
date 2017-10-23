#pragma once

#include <stdint.h>
#include <tools/args_parser.h>
#include <kernel/mdb_kernel.h>
#include <sched/rsched.h>

int render_run(struct rsched* sched, struct mdb_kernel* kernel,
               struct surface* surf, uint32_t width, uint32_t height,
               bool color_enabled);
