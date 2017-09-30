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
    surface* surf;
    float width_r;
    float height_r;
    float aspect_ratio;
} mdb;

static const char* name = "Mandelbrot native kernel";
static const char* ver_maj = "1";
static const char* ver_min = "0";

__export_sym
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

__export_sym
int mdb_kernel_cpu_features(void)
{
    return 0;
}

__export_sym
void mdb_kernel_init(void)
{

}

__export_sym
void mdb_kernel_shutdown(void)
{

}

__export_sym
void mdb_kernel_set_size(uint32_t width, uint32_t height)
{
    mdb.width = width;
    mdb.height = height;
    mdb.width_r = 1.0f / width;
    mdb.height_r = 1.0f / height;
    mdb.aspect_ratio = (float)width / height;
}

__export_sym
void mdb_kernel_set_scale(float scale)
{
    mdb.scale = scale;
}

__export_sym
void mdb_kernel_set_shift(float shift_x, float shift_y)
{
    mdb.shift_x = shift_x;
    mdb.shift_y = shift_y;
}

__export_sym
void mdb_kernel_set_bailout(uint32_t bailout)
{
    mdb.bailout = bailout;
}

__export_sym
void mdb_kernel_set_surface(surface* surf)
{
    mdb.surf = surf;
}

__export_sym
void mdb_kernel_submit_changes(void)
{
}

__export_sym
MDB_KERNEL_DEFINE_PROCESS_BLOCK_FUN(mdb_kernel_process_block)
