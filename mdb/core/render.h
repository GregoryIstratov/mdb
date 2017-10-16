#pragma once

#include <stdint.h>
#include <mdb/tools/args_parser.h>
#include <mdb/kernel/mdb_kernel.h>
#include <mdb/sched/rsched.h>


int render_run(struct rsched* sched, mdb_kernel* kernel, surface* surf, uint32_t width, uint32_t height, bool color_enabled);