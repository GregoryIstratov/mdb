#include "mdb_kernel.h"
#include <stdint.h>
#include <immintrin.h>
#include <stdalign.h>
#include <mdb/tools/utils.h>
#include <mdb/kernel/bits/mdb_kernel.h>

int mdb_kernel_create(mdb_kernel** pmdb, int kernel_type, int width, int height, int bailout)
{
    *pmdb = calloc(1, sizeof(mdb_kernel));
    mdb_kernel* mdb = *pmdb;

    mdb->bailout = bailout;
    mdb->width = width;
    mdb->height = height;
    mdb->scale = MDB_FLOAT_C(0.00188964);
    mdb->shift_x = MDB_FLOAT_C(-1.347385054652062);
    mdb->shift_y = MDB_FLOAT_C(0.063483549665202);
    mdb->width_r = MDB_FLOAT_C(1.0) / width;
    mdb->height_r = MDB_FLOAT_C(1.0) / height;
    mdb->aspect_ratio = MDB_FLOAT_C(width) / MDB_FLOAT_C(height);
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
                LOG_ERROR("Your CPU doesn't support needed features [avx2,fma] to run this kernel")
                return -1;
            }
#else
            LOG_ERROR("Kernel [avx2_fma] is not enabled at build.")
            return -1;
#endif
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
                LOG_ERROR("Your CPU doesn't support needed features [avx2] to run this kernel")
                return -1;
            }
#else
            LOG_ERROR("Kernel [avx2] is not enabled at build.")
            return -1;
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
            LOG_ERROR("Kernel [native] is not enabled at build.")
            return -1;
#endif
        }

        default:
            return -1;
    }

    return 0;
}

void mdb_kernel_destroy(mdb_kernel* mdb)
{
    free(mdb);
}


void mdb_kernel_set_surface(mdb_kernel* mdb, float* f32surface)
{
    mdb->f32surface = f32surface;
}

void mdb_kernel_set_size(mdb_kernel* mdb, uint32_t width, uint32_t height)
{
    mdb->width = width;
    mdb->height = height;
    mdb->width_r = MDB_FLOAT_C(1.0) / width;
    mdb->height_r = MDB_FLOAT_C(1.0) / height;
    mdb->aspect_ratio = MDB_FLOAT_C(width) / MDB_FLOAT_C(height);
}

void mdb_kernel_set_scale(mdb_kernel* mdb, float scale)
{
    mdb->scale = scale;
}

void mdb_kernel_set_shift(mdb_kernel* mdb, float shift_x, float shift_y)
{
    mdb->shift_x = shift_x;
    mdb->shift_y = shift_y;
}

void mdb_kernel_set_bailout(mdb_kernel* mdb, uint32_t bailout)
{
    mdb->bailout = bailout;
}

void mdb_kernel_submit_changes(mdb_kernel* mdb)
{
    ((void)mdb);
}

void mdb_kernel_process_block(mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1)
{
    mdb->block_fun(mdb, x0, x1, y0, y1);
}
