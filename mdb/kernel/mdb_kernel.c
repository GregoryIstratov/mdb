#include "mdb_kernel.h"
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <stdint.h>
#include <stdalign.h>

#include <immintrin.h>

#include <mdb/tools/utils.h>
#include <mdb/tools/log.h>

#include <mdb/kernel/bits/mdb_kernel.h>
#include <mdb/kernel/asm/mdb_asm_kernel.h>



static void mdb_kernel_init(mdb_kernel* mdb);

static int mdb_kernel_ext_load(mdb_kernel* mdb, const char* ext_kernel)
{

#define KRN_CHECK_RESOLV  \
    if ((error = dlerror()) != NULL) \
    { \
        LOG_ERROR("Error on symbol resolving: %s", error); \
        return -1; \
    }

    if(!ext_kernel)
    {
        LOG_ERROR("Kernel name is not set.");
        return -1;
    }

    char filename[1024];
    memset(filename, 0, 1024);

    strcpy(filename, "./kernels/");
    strcat(filename, ext_kernel);
    strcat(filename, ".so");

    LOG_SAY("Loading external kernel: %s ...", filename);

    void* handle;
    char* error;

    handle = dlopen(filename, RTLD_LAZY);
    if(!handle)
    {
        LOG_ERROR("Failed to load kernel '%s': %s", ext_kernel, dlerror());
        return -1;
    }

    /* Clear any existing error */
    dlerror();

    mdb->ext_metadata_query_fun = (mdb_kernel_ext_metadata_query_t)dlsym(handle, "mdb_kernel_ext_metadata_query");

    KRN_CHECK_RESOLV

    mdb->ext_init_fun = (mdb_kernel_ext_init_t)dlsym(handle, "mdb_kernel_ext_init");

    KRN_CHECK_RESOLV

    mdb->ext_shutdown_fun = (mdb_kernel_ext_shutdown_t)dlsym(handle, "mdb_kernel_ext_shutdown");

    KRN_CHECK_RESOLV

    mdb->ext_block_fun = (mdb_kernel_ext_process_block_t)dlsym(handle, "mdb_kernel_ext_process_block");

    KRN_CHECK_RESOLV

    mdb->ext_set_size_fun = (mdb_kernel_ext_set_size_t)dlsym(handle, "mdb_kernel_ext_set_size");

    KRN_CHECK_RESOLV

    mdb->ext_set_scale_fun = (mdb_kernel_ext_set_scale_t)dlsym(handle, "mdb_kernel_ext_set_scale");

    KRN_CHECK_RESOLV

    mdb->ext_set_shift_fun = (mdb_kernel_ext_set_shift_t)dlsym(handle, "mdb_kernel_ext_set_shift");

    KRN_CHECK_RESOLV

    mdb->ext_set_bailout_fun = (mdb_kernel_ext_set_bailout_t)dlsym(handle, "mdb_kernel_ext_set_bailout");

    KRN_CHECK_RESOLV

    mdb->ext_set_surface_fun = (mdb_kernel_ext_set_surface_t)dlsym(handle, "mdb_kernel_ext_set_surface");

    KRN_CHECK_RESOLV

    mdb->ext_submit_changes_fun = (mdb_kernel_ext_submit_changes_t)dlsym(handle, "mdb_kernel_ext_submit_changes");

    KRN_CHECK_RESOLV

#undef KRN_CHECK_RESOLV

    mdb->dl_handle = handle;

    LOG_SAY("External kernel '%s' has been successfully loaded.", ext_kernel);

    return 0;
}

static void mdb_kernel_ext_log_info(mdb_kernel* mdb)
{
    char buff[256];
    memset(buff, 0, 256);

    LOG_SAY("==External kernel metadata info==");

    mdb->ext_metadata_query_fun(MDB_KERNEL_EXT_META_NAME, buff, 256);
    PARAM_INFO("Name", "%s", buff);

    mdb->ext_metadata_query_fun(MDB_KERNEL_EXT_META_VER_MAJ, buff, 256);
    PARAM_INFO("Version major", "%s", buff);

    mdb->ext_metadata_query_fun(MDB_KERNEL_EXT_META_VER_MIN, buff, 256);
    PARAM_INFO("Version minor", "%s", buff);

    LOG_SAY("---------------------------------");
}

int mdb_kernel_create(mdb_kernel** pmdb, int kernel_type, const char* ext_kernel)
{
    *pmdb = calloc(1, sizeof(mdb_kernel));
    mdb_kernel* mdb = *pmdb;

    mdb->kernel_type = kernel_type;

    switch (kernel_type)
    {
        case MDB_KERNEL_AVX2_FMA:
        {
#if defined(MDB_ENABLE_AVX2_FMA_KERNEL)
            if(CPU_CHECK_FEATURE("avx2") && CPU_CHECK_FEATURE("fma"))
            {
                mdb->block_fun = &mdb_kernel_process_block_avx2_fma;
                break;
            }
            else
            {
                LOG_ERROR("Your CPU doesn't support needed features [avx2,fma] to run this kernel");
                goto error_exit;
            }
#else
            LOG_ERROR("Kernel [avx2_fma] is not enabled at build.");
            goto error_exit;
#endif
        }

        case MDB_KERNEL_AVX2_FMA_ASM:
        {
            mdb->kernel_type = MDB_KERNEL_EXTERNAL;

            mdb->ext_metadata_query_fun = &mdb_asm_kernel_metadata_query;
            mdb->ext_init_fun = &mdb_asm_kernel_init;
            mdb->ext_shutdown_fun = &mdb_asm_kernel_shutdown;
            mdb->ext_block_fun = &mdb_asm_kernel_process_block;
            mdb->ext_set_bailout_fun = &mdb_asm_kernel_set_bailout;
            mdb->ext_set_scale_fun = &mdb_asm_kernel_set_scale;
            mdb->ext_set_shift_fun = &mdb_asm_kernel_set_shift;
            mdb->ext_set_size_fun = &mdb_asm_kernel_set_size;
            mdb->ext_set_surface_fun = &mdb_asm_kernel_set_surface;
            mdb->ext_submit_changes_fun = &mdb_asm_kernel_submit_changes;

            mdb_kernel_ext_log_info(mdb);
            break;
        }

        case MDB_KERNEL_AVX2:
        {
#if defined(MDB_ENABLE_AVX2_KERNEL)
            if(CPU_CHECK_FEATURE("avx2"))
            {
                mdb->block_fun = &mdb_kernel_process_block_avx2;
                break;
            }
            else
            {
                LOG_ERROR("Your CPU doesn't support needed features [avx2] to run this kernel");
                goto error_exit;
            }
#else
            LOG_ERROR("Kernel [avx2] is not enabled at build.");
            goto error_exit;
#endif
        }

        case MDB_KERNEL_GENERIC:
            mdb->block_fun = &mdb_kernel_process_block_generic;
            break;

        case MDB_KERNEL_NATIVE:
        {
#if defined(MDB_ENABLE_NATIVE_KERNEL)
            mdb->block_fun = &mdb_kernel_process_block_native;
            break;
#else
            LOG_ERROR("Kernel [native] is not enabled at build.");
            goto error_exit;
#endif
        }

        case MDB_KERNEL_EXTERNAL:
        {
            if(mdb_kernel_ext_load(mdb, ext_kernel) != 0)
                goto error_exit;

            mdb_kernel_ext_log_info(mdb);

            break;
        }

        default:
            goto error_exit;
    }

    mdb_kernel_init(mdb);
    mdb_kernel_set_bailout(mdb, 256);
    mdb_kernel_set_size(mdb, 1024, 1024);
    mdb_kernel_set_scale(mdb, MDB_FLOAT_C(0.00188964));
    mdb_kernel_set_shift(mdb, MDB_FLOAT_C(-1.347385054652062), MDB_FLOAT_C(0.063483549665202));
    mdb_kernel_submit_changes(mdb);

    return 0;

error_exit:
    free(*pmdb);
    *pmdb = NULL;
    return -1;
}


static void mdb_kernel_init(mdb_kernel* mdb)
{
    if(mdb->kernel_type == MDB_KERNEL_EXTERNAL)
    {
        mdb->ext_init_fun();
    }
}

void mdb_kernel_destroy(mdb_kernel* mdb)
{
    if(mdb->kernel_type == MDB_KERNEL_EXTERNAL)
    {
        mdb->ext_shutdown_fun();

        if(mdb->dl_handle)
        {
            dlclose(mdb->dl_handle);
            mdb->dl_handle = NULL;
        }
    }

    free(mdb);
}


void mdb_kernel_set_surface(mdb_kernel* mdb, float* f32surface)
{
    if(mdb->kernel_type != MDB_KERNEL_EXTERNAL)
        mdb->f32surface = f32surface;
    else
        mdb->ext_set_surface_fun(f32surface);
}

void mdb_kernel_set_size(mdb_kernel* mdb, uint32_t width, uint32_t height)
{
    if(mdb->kernel_type != MDB_KERNEL_EXTERNAL)
    {
        mdb->width = width;
        mdb->height = height;
        mdb->width_r = MDB_FLOAT_C(1.0) / width;
        mdb->height_r = MDB_FLOAT_C(1.0) / height;
        mdb->aspect_ratio = MDB_FLOAT_C(width) / MDB_FLOAT_C(height);
    }
    else
        mdb->ext_set_size_fun(width, height);
}

void mdb_kernel_set_scale(mdb_kernel* mdb, float scale)
{
    if(mdb->kernel_type != MDB_KERNEL_EXTERNAL)
        mdb->scale = scale;
    else
        mdb->ext_set_scale_fun(scale);
}

void mdb_kernel_set_shift(mdb_kernel* mdb, float shift_x, float shift_y)
{
    if(mdb->kernel_type != MDB_KERNEL_EXTERNAL)
    {
        mdb->shift_x = shift_x;
        mdb->shift_y = shift_y;
    }
    else
        mdb->ext_set_shift_fun(shift_x, shift_y);
}

void mdb_kernel_set_bailout(mdb_kernel* mdb, uint32_t bailout)
{
    if(mdb->kernel_type != MDB_KERNEL_EXTERNAL)
        mdb->bailout = bailout;
    else
        mdb->ext_set_bailout_fun(bailout);
}

void mdb_kernel_submit_changes(mdb_kernel* mdb)
{
    if(mdb->kernel_type == MDB_KERNEL_EXTERNAL)
        mdb->ext_submit_changes_fun();
}

void mdb_kernel_process_block(mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1)
{
    if(mdb->kernel_type != MDB_KERNEL_EXTERNAL)
        mdb->block_fun(mdb, x0, x1, y0, y1);
    else
        mdb->ext_block_fun(x0, x1, y0, y1);
}
