#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/sysinfo.h>

#include <mdb/tools/args_parser.h>
#include <mdb/kernel/mdb_kernel.h>
#include <string.h>
#include <mdb/app/benchmark.h>
#include <mdb/app/render.h>
#include <mdb/surface/surface.h>
#include <mdb/tools/log.h>

#include <mdb/tools/image_hdr.h>
#include <locale.h>
#include <mdb/tools/cpu_features.h>
#include <mdb/tools/error_codes.h>
#include <mdb/tools/timer.h>

static inline
const char* mode_str(int mode)
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

static inline
uint32_t get_nthreads(void)
{
        int nproc = get_nprocs_conf();
        int nproc_avail = get_nprocs();

        LOG_VINFO(LOG_VERBOSE1, "This system has %d processors configured "
                 "and %d processors available.",
                 nproc, nproc_avail);

        return nproc_avail;
}

static
void print_input_params(struct arguments* args)
{
        const char* mode_s = mode_str(args->mode);


        PARAM_INFO("Run mode", "%s", mode_s);

        if(args->mode == MODE_BENCHMARK)
                PARAM_INFO("Benchmark runs", "%i", args->benchmark_runs);

        PARAM_INFO("Kernel", "%s", args->kernel_name);

        if(args->threads == -1)
                PARAM_INFO("Threads", "%s", "auto");
        else
                PARAM_INFO("Threads", "%d", args->threads);

        PARAM_INFO("Block size", "%ix%i", args->block_size_x, args->block_size_y);
        PARAM_INFO("Width", "%i", args->width);
        PARAM_INFO("Height", "%i", args->height);
        PARAM_INFO("Bailout", "%i", args->bailout);
}

static
int run_benchmark_mode(struct mdb_kernel* kernel,
                       struct rsched* sched, struct arguments* args)
{
        struct surface* surf;
        struct benchmark* bench;
        uint32_t runs;
        int ret;

        ret = surface_create(&surf, args->width, args->height,
                             SURFACE_BUFFER_CREATE | SURFACE_BUFFER_F32);

        if(ret != MDB_SUCCESS)
        {
                LOG_ERROR("Cannot create surface.");
                return ret;
        }

        mdb_kernel_set_size(kernel, args->width, args->height);
        mdb_kernel_set_surface(kernel, surf);

        runs = args->mode == MODE_ONESHOT ? 1 : args->benchmark_runs;
        benchmark_create(&bench,
                         runs,
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
                        LOG_ERROR("Failed to save surface to '%s', errno: %s",
                                  args->output_file, strerror(errno));
                else
                        LOG_SAY("Image saved to '%s'", args->output_file);

        }


        benchmark_destroy(bench);
        surface_destroy(surf);

        return MDB_SUCCESS;
}

static
int run_render_mode(struct mdb_kernel* kernel, struct rsched* sched,
                    struct arguments* args)
{
        struct surface* surf;
        int ret;

        ret = surface_create(&surf, args->width, args->height,
                             SURFACE_BUFFER_EXT | SURFACE_BUFFER_F32);

        if(ret != MDB_SUCCESS)
        {
                LOG_ERROR("Cannot create surface.");
                return ret;
        }

        mdb_kernel_set_size(kernel, args->width, args->height);
        mdb_kernel_set_surface(kernel, surf);


        ret = render_run(sched, kernel, surf, args->width, args->height,
                         args->shader_colors? true : false);

        if(ret != MDB_SUCCESS)
        {
                LOG_ERROR("Failed to run render.");
        }

        surface_destroy(surf);

        return ret;
}

static
void configure_rsched_options(struct rsched_options* opts,
                              struct arguments* args)
{
        if(args->threads <= -1)
                opts->threads = get_nthreads();
        else
                opts->threads = (uint32_t)args->threads;


#if defined(CONFIG_RSCHED_PROFILE)
        opts->profile.run_hist.show =
                optional_get(&args->rsched.run_hist.show, true);

        opts->profile.run_hist.log_scale =
                optional_get(&args->rsched.run_hist.log_scale, false);

        opts->profile.run_hist.size =
                optional_get(&args->rsched.run_hist.size, 8);

        opts->profile.run_hist.min =
                optional_get(&args->rsched.run_hist.min, 1 * NS_IN_MS);

        opts->profile.run_hist.max =
                optional_get(&args->rsched.run_hist.max, 30 * NS_IN_MS);


        opts->profile.task_hist.show =
                optional_get(&args->rsched.task_hist.show, true);

        opts->profile.task_hist.log_scale =
                optional_get(&args->rsched.task_hist.log_scale, false);

        opts->profile.task_hist.size =
                optional_get(&args->rsched.task_hist.size, 16);

        opts->profile.task_hist.min =
                optional_get(&args->rsched.task_hist.min, 20 * NS_IN_MCS);

        opts->profile.task_hist.max =
                optional_get(&args->rsched.task_hist.max, 143 * NS_IN_MCS);


        memcpy(&opts->profile.payload_hist, &opts->profile.task_hist,
               sizeof(opts->profile.task_hist));

#endif
}

int main(int argc, char** argv)
{
        struct arguments args;

        struct mdb_kernel* kernel;
        struct rsched* sched;
        struct rsched_options rsched_opts = {0};
        struct block_size block_size;

        bool exit_failure = false;

        /* Set up default locale
         * mostly for formatting printf output */
        setlocale(LC_ALL, "");

        args_parse(argc, argv, &args);

        log_init(args.silent? LOGLEVEL_NONE : LOGLEVEL_ALL,
                 args.verbose,
                 NULL);


        if(mdb_kernel_create(&kernel, args.kernel_name) != MDB_SUCCESS)
        {
                LOG_ERROR("Cannot create the kernel");
                log_shutdown();
                exit(EXIT_FAILURE);
        }


        print_input_params(&args);

        block_size.x = args.block_size_x;
        block_size.y = args.block_size_y;

        configure_rsched_options(&rsched_opts, &args);

        rsched_create(&sched, &rsched_opts);
        rsched_tune_thread_affinity(sched);
        rsched_create_tasks(sched, (uint32_t) args.width, (uint32_t) args.height,
                            &block_size);

        switch(args.mode)
        {
        case MODE_BENCHMARK:
        case MODE_ONESHOT:
                if(run_benchmark_mode(kernel, sched, &args) != MDB_SUCCESS)
                {
                        exit_failure = true;
                        goto shutdown;
                }
                break;
        case MODE_RENDER:
                if(run_render_mode(kernel, sched, &args) != MDB_SUCCESS)
                {
                        exit_failure = true;
                        goto shutdown;
                }
                break;
        default:
                LOG_ERROR("Unknown run mode %i", args.mode);
                exit_failure = true;
                goto shutdown;
        }


shutdown:
        rsched_print_stats(sched);
        rsched_shutdown(sched);
        mdb_kernel_destroy(kernel);
        log_shutdown();

        if(exit_failure)
                exit(EXIT_FAILURE);

        return 0;
}