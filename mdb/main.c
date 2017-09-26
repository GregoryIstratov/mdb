#include <stdlib.h>
#include <errno.h>
#include <sys/sysinfo.h>

#include <mdb/tools/args_parser.h>
#include <mdb/kernel/mdb_kernel.h>
#include <string.h>
#include <mdb/core/benchmark.h>
#include <mdb/core/render.h>
#include <mdb/tools/log.h>

#include <mdb/tools/image/image_hdr.h>
#include <locale.h>

static const char* render_control_keys =
"Arrows  - Move Up/Down/Left/Right\n"
"[1]/[2] - Scale up/down\n"
"[3]/[4] - Iterations/bailout increase/decrease\n"
"[5]/[6] - Exposure increase/decrease\n";


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

inline static const char* kernel_type_str(int kernel_type)
{
    switch(kernel_type)
    {
        case MDB_KERNEL_GENERIC:
            return "generic";
        case MDB_KERNEL_NATIVE:
            return "native";
        case MDB_KERNEL_AVX2:
            return "avx2";
        case MDB_KERNEL_AVX2_FMA:
            return "avx2_fma";
        case MDB_KERNEL_AVX2_FMA_ASM:
            return "avx2_fma_asm";
        case MDB_KERNEL_EXTERNAL:
            return "external";

        default:
            return "unknown";
    }
}

static float* surface_create(int width, int height)
{
    float* surface = NULL;
    size_t size = (size_t) (width * height);
    size_t mem_size = size * sizeof(float);
    size_t align = 4096; //force to allocate it on a new page

    int res = posix_memalign((void**) &surface, align, mem_size);
    if (res)
    {
        if (ENOMEM == res)
            LOG_ERROR("There was insufficient memory available to satisfy the request.");
        if (EINVAL == res)
            LOG_ERROR("Alignment is not a power of two multiple of sizeof (void *).");

        exit(EXIT_FAILURE);
    }

    memset(surface, 0, mem_size);

    return surface;
}

static void surface_destroy(float* surface)
{
    free(surface);
}


int main(int argc, char** argv)
{
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
    PARAM_INFO("Kernel", "%s", kernel_type_str(args.kernel_type));
    PARAM_INFO("Threads", "%i", threads);
    PARAM_INFO("Block size", "%ix%i", args.block_size.x, args.block_size.y);
    PARAM_INFO("Width", "%i", args.width);
    PARAM_INFO("Height", "%i", args.height);
    PARAM_INFO("Bailout", "%i", args.bailout);

    mdb_kernel* kernel;
    if(mdb_kernel_create(&kernel, args.kernel_type, args.kernel_name) != 0)
    {
        LOG_ERROR("Cannot create the kernel");
        log_shutdown();
        exit(EXIT_FAILURE);
    }

    rsched* sched;
    rsched_create(&sched, (uint32_t) threads);
    rsched_create_tasks(sched, (uint32_t) args.width, (uint32_t) args.height, &args.block_size);
    LOG_DEBUG("Enqueued tasks %u", rsched_enqueued_tasks(sched));

    if(args.mode == MODE_BENCHMARK || args.mode == MODE_ONESHOT)
    {
        float* surface = surface_create(args.width, args.height);

        mdb_kernel_set_surface(kernel, surface);
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
            if (image_hdr_save_r32(args.output_file, args.width, args.height, surface))
            {
                LOG_ERROR("Failed to save surface to '%s', errno: %s", args.output_file, strerror(errno));
            }
            else
            {
                LOG_SAY("Image saved to '%s'", args.output_file);
            }
        }

        surface_destroy(surface);

    }
    else if(args.mode == MODE_RENDER)
    {
#if defined(OGL_RENDER_ENABLED)
        LOG_SAY("Starting render mode...\nControl keys:\n%s\n", render_control_keys);
        render_run(sched, kernel, &args);
#else
        UNUSED_PARAM(render_control_keys);
        LOG_ERROR("OGL Render disabled at the build time.");
#endif
    }
    else
    {
        LOG_ERROR("Unknown run mode %i", args.mode);
    }

    rsched_shutdown(sched);
    mdb_kernel_destroy(kernel);
    log_shutdown();

    return 0;
}