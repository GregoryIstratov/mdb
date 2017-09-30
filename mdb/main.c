#include <stdlib.h>
#include <errno.h>
#include <sys/sysinfo.h>

#include <mdb/tools/args_parser.h>
#include <mdb/kernel/mdb_kernel.h>
#include <string.h>
#include <mdb/core/benchmark.h>
#include <mdb/core/render.h>
#include <mdb/surface/surface.h>
#include <mdb/tools/log.h>

#include <mdb/tools/image/image_hdr.h>
#include <locale.h>

inline static const char* mode_str(int mode)
{
    switch(mode)
    {
        case MODE_ONESHOT:
            return "oneshot";
        case MODE_BENCHMARK:
            return "benchmark";
        case MODE_RENDER:
            return "render";

        default:
            return "unknown";
    }
}

int main(int argc, char** argv)
{
    bool exit_failure = false;

    setlocale(LC_ALL, "");

    struct arguments args;
    args_parse(argc, argv, &args);

    log_init(LOGLEVEL_ALL, NULL);

    int threads;
    if(args.threads <= -1)
    {
        int nproc = get_nprocs_conf();
        int nproc_avail = get_nprocs();
        LOG_SAY("This system has %d processors configured and %d processors available.",
               nproc, nproc_avail);

        threads = (uint32_t) get_nprocs();
    }
    else
        threads = (uint32_t)args.threads;

    PARAM_INFO("Run mode", "%s", mode_str(args.mode));
    if(args.mode == MODE_BENCHMARK)
        PARAM_INFO("Benchmark runs", "%i", args.benchmark_runs);
    PARAM_INFO("Kernel", "%s", args.kernel_name);
    PARAM_INFO("Threads", "%i", threads);
    PARAM_INFO("Block size", "%ix%i", args.block_size.x, args.block_size.y);
    PARAM_INFO("Width", "%i", args.width);
    PARAM_INFO("Height", "%i", args.height);
    PARAM_INFO("Bailout", "%i", args.bailout);

    mdb_kernel* kernel;
    if(mdb_kernel_create(&kernel, args.kernel_name) != 0)
    {
        LOG_ERROR("Cannot create the kernel");
        log_shutdown();
        exit(EXIT_FAILURE);
    }

    rsched* sched;
    rsched_create(&sched, (uint32_t) threads);
    rsched_create_tasks(sched, (uint32_t) args.width, (uint32_t) args.height, &args.block_size);
    LOG_DEBUG("Enqueued tasks %u", rsched_enqueued_tasks(sched));

    surface* surf = NULL;

    if(args.mode == MODE_BENCHMARK || args.mode == MODE_ONESHOT)
    {
        if(surface_create(&surf, args.width, args.height, SURFACE_BUFFER_CREATE | SURFACE_BUFFER_F32) != 0)
        {
            LOG_ERROR("Cannot create surface.");
            exit_failure = true;
            goto shutdown;
        }

        mdb_kernel_set_surface(kernel, surf);
        mdb_kernel_set_bailout(kernel, (uint32_t) args.bailout);
        mdb_kernel_set_size(kernel, (uint32_t) args.width, (uint32_t) args.height);

        benchmark* bench = NULL;
        uint32_t runs = args.mode == MODE_ONESHOT ? 1 : (uint32_t)args.benchmark_runs;
        benchmark_create(&bench, runs, kernel, sched);

        if(args.mode == MODE_BENCHMARK)
            LOG_SAY("Running benchmark...");

        benchmark_run(bench);

        benchmark_print_summary(bench);

        if(args.mode == MODE_ONESHOT)
        {
            errno = 0;
            if (surface_save_image_hdr(surf, args.output_file))
            {
                LOG_ERROR("Failed to save surface to '%s', errno: %s", args.output_file, strerror(errno));
            }
            else
            {
                LOG_SAY("Image saved to '%s'", args.output_file);
            }
        }
    }
    else if(args.mode == MODE_RENDER)
    {
        if(surface_create(&surf, args.width, args.height, SURFACE_BUFFER_EXT | SURFACE_BUFFER_F32) != 0)
        {
            LOG_ERROR("Cannot create surface.");
            exit_failure = true;
            goto shutdown;
        }

        mdb_kernel_set_surface(kernel, surf);

       if(render_run(sched, kernel, surf, args.width, args.height))
       {
           LOG_ERROR("Failed to run render.");
           exit_failure = true;
           goto shutdown;
       }

    }
    else
    {
        LOG_ERROR("Unknown run mode %i", args.mode);
    }

shutdown:
    surface_destroy(surf);
    rsched_shutdown(sched);
    mdb_kernel_destroy(kernel);
    log_shutdown();

    if(exit_failure)
        exit(EXIT_FAILURE);

    return 0;
}