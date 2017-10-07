#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
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
#include <mdb/tools/cpu_features.h>
#include <mdb/tools/error_codes.h>

static const char* mode_str(int mode)
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

static uint32_t get_nthreads(void)
{
    int nproc = get_nprocs_conf();
    int nproc_avail = get_nprocs();

    LOG_INFO("This system has %d processors configured and %d processors available.",
            nproc, nproc_avail);

    return (uint32_t)nproc_avail;
}

static void print_input_params(struct arguments* args, uint32_t threads)
{
    const char* mode_s = mode_str(args->mode);
    PARAM_INFO("Run mode", "%s", mode_s);
    if(args->mode == MODE_BENCHMARK)
        PARAM_INFO("Benchmark runs", "%i", args->benchmark_runs);
    PARAM_INFO("Kernel", "%s", args->kernel_name);
    PARAM_INFO("Threads", "%i", threads);
    PARAM_INFO("Block size", "%ix%i", args->block_size.x, args->block_size.y);
    PARAM_INFO("Width", "%i", args->width);
    PARAM_INFO("Height", "%i", args->height);
    PARAM_INFO("Bailout", "%i", args->bailout);
}

static int run_benchmark_mode(mdb_kernel* kernel, rsched* sched, struct arguments* args)
{
    surface* surf;
    benchmark* bench;

    if(surface_create(&surf, args->width, args->height, SURFACE_BUFFER_CREATE | SURFACE_BUFFER_F32) != MDB_SUCCESS)
    {
        LOG_ERROR("Cannot create surface.");
        return MDB_FAIL;
    }

    mdb_kernel_set_size(kernel, (uint32_t) args->width, (uint32_t) args->height);
    mdb_kernel_set_surface(kernel, surf);

    benchmark_create(&bench,
                     args->mode == MODE_ONESHOT ? 1 : (uint32_t)args->benchmark_runs,
                     kernel,
                     sched);

    if(args->mode == MODE_BENCHMARK)
        LOG_SAY("Running benchmark...");

    benchmark_run(bench);

    benchmark_print_summary(bench);

    if(args->mode == MODE_ONESHOT)
    {
        errno = 0;
        if (surface_save_image_hdr(surf, args->output_file))
        {
            LOG_ERROR("Failed to save surface to '%s', errno: %s", args->output_file, strerror(errno));
        }
        else
        {
            LOG_SAY("Image saved to '%s'", args->output_file);
        }
    }


    benchmark_destroy(bench);
    surface_destroy(surf);

    return MDB_SUCCESS;
}

static int run_render_mode(mdb_kernel* kernel, rsched* sched, struct arguments* args)
{
    surface* surf;
    int ret = MDB_SUCCESS;

    if(surface_create(&surf, args->width, args->height, SURFACE_BUFFER_EXT | SURFACE_BUFFER_F32) != MDB_SUCCESS)
    {
        LOG_ERROR("Cannot create surface.");
        return MDB_FAIL;
    }

    mdb_kernel_set_surface(kernel, surf);

    if(render_run(sched, kernel, surf, args->width, args->height, args->shader_colors? true : false) != MDB_SUCCESS)
    {
        LOG_ERROR("Failed to run render.");

        ret = MDB_FAIL;

        goto exit;
    }

exit:
    surface_destroy(surf);

    return ret;
}

int main(int argc, char** argv)
{
    struct arguments args;
    uint32_t threads;

    mdb_kernel* kernel;
    rsched* sched;

    bool exit_failure = false;

    /* Set up default locale
     * mostly for formatting printf output */
    setlocale(LC_ALL, "");

    args_parse(argc, argv, &args);

    log_init(LOGLEVEL_ALL, NULL);

    if(args.threads <= -1)
        threads = get_nthreads();
    else
        threads = (uint32_t)args.threads;

    print_input_params(&args, threads);


    if(mdb_kernel_create(&kernel, args.kernel_name) != MDB_SUCCESS)
    {
        LOG_ERROR("Cannot create the kernel");
        log_shutdown();
        exit(EXIT_FAILURE);
    }


    rsched_create(&sched, threads);
    rsched_create_tasks(sched, (uint32_t) args.width, (uint32_t) args.height, &args.block_size);

    LOG_DEBUG("Enqueued tasks %u", rsched_enqueued_tasks(sched));

    if(args.mode == MODE_BENCHMARK || args.mode == MODE_ONESHOT)
    {
        if(run_benchmark_mode(kernel, sched, &args) != MDB_SUCCESS)
        {
            exit_failure = true;
            goto shutdown;
        }
    }
    else if(args.mode == MODE_RENDER)
    {
        if(run_render_mode(kernel, sched, &args) != MDB_SUCCESS)
        {
            exit_failure = true;
            goto shutdown;
        }
    }
    else
    {
        LOG_ERROR("Unknown run mode %i", args.mode);
        exit_failure = true;
        goto shutdown;
    }

shutdown:
    rsched_shutdown(sched);
    mdb_kernel_destroy(kernel);
    log_shutdown();

    if(exit_failure)
        exit(EXIT_FAILURE);

    return 0;
}