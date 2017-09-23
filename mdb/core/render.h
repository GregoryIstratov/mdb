#pragma once

#include <stdint.h>
#include <mdb/tools/args_parser.h>
#include <mdb/kernel/mdb_kernel.h>
#include <mdb/sched/rsched.h>


void render_run(rsched* sched, mdb_kernel* kernel, struct arguments* args);