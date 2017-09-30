#pragma once

#include <stdint.h>
#include <mdb/tools/args_parser.h>
#include <mdb/kernel/mdb_kernel.h>
#include <mdb/sched/rsched.h>


int render_run(rsched* sched, mdb_kernel* kernel, surface* surf, uint32_t width, uint32_t height);