#include "kernel_common.h"
#include <kernel_module.h>

static struct
{
    uint32_t bailout;
    uint32_t width;
    uint32_t height;
    float shift_x;
    float shift_y;
    float scale;
    float* f32surface;
    float width_r;
    float height_r;
    float aspect_ratio;
} mdb;

static const char* name = "Mandelbrot generic kernel";
static const char* ver_maj = "1";
static const char* ver_min = "0";

int mdb_kernel_metadata_query(int query, char* buff, uint32_t buff_size)
{
    switch(query)
    {
        case MDB_KERNEL_META_NAME:
            return metadata_copy(name, buff, buff_size);

        case MDB_KERNEL_META_VER_MAJ:
            return metadata_copy(ver_maj, buff, buff_size);

        case MDB_KERNEL_META_VER_MIN:
            return metadata_copy(ver_min, buff, buff_size);

        default:
            return MDB_QUERY_NO_PARAM;
    }
}

int mdb_kernel_cpu_features(void)
{
    return 0;
}

void mdb_kernel_init(void)
{

}

void mdb_kernel_shutdown(void)
{

}

void mdb_kernel_set_size(uint32_t width, uint32_t height)
{
    mdb.width = width;
    mdb.height = height;
    mdb.width_r = 1.0f / width;
    mdb.height_r = 1.0f / height;
    mdb.aspect_ratio = (float)width / height;
}

void mdb_kernel_set_scale(float scale)
{
    mdb.scale = scale;
}

void mdb_kernel_set_shift(float shift_x, float shift_y)
{
    mdb.shift_x = shift_x;
    mdb.shift_y = shift_y;
}


void mdb_kernel_set_bailout(uint32_t bailout)
{
    mdb.bailout = bailout;
}

void mdb_kernel_set_surface(float* buffer)
{
    mdb.f32surface = buffer;
}

void mdb_kernel_submit_changes(void)
{
}

/* Override default compiler options for generating common generic code
 * without any vector extension like sse,avx,fma, etc. and force it to use x87 math coprocessor instead of default sse
 */
__attribute__((target("arch=x86-64,tune=generic,no-mmx,no-sse,no-sse2,no-sse3,no-sse4,no-avx,no-avx2,no-fma,fpmath=387")))
MDB_KERNEL_DEFINE_PROCESS_BLOCK_FUN(mdb_kernel_process_block)
