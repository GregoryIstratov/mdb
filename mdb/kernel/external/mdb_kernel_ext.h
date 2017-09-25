#pragma once
#include <stdlib.h>
#include <stdint.h>

enum
{
    MDB_KERNEL_EXT_META_NAME    = 1,
    MDB_KERNEL_EXT_META_VER_MAJ = 1 << 1,
    MDB_KERNEL_EXT_META_VER_MIN = 1 << 2,
};

typedef void (*mdb_kernel_ext_metadata_query_t)(int flags, char* buff, uint32_t buff_size);

typedef void (*mdb_kernel_ext_init_t)(void);

typedef void (*mdb_kernel_ext_shutdown_t)(void);

typedef void (*mdb_kernel_ext_process_block_t)(uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);

typedef void (*mdb_kernel_ext_set_size_t)(uint32_t width, uint32_t height);

typedef void (*mdb_kernel_ext_set_scale_t)(float scale);

typedef void (*mdb_kernel_ext_set_shift_t)(float shift_x, float shift_y);

typedef void (*mdb_kernel_ext_set_bailout_t)(uint32_t bailout);

typedef void (*mdb_kernel_ext_set_surface_t)(float* buffer);

typedef void (*mdb_kernel_ext_submit_changes_t)(void);

__always_inline static char* mdb_metadata_next(char* s, uint32_t* sz)
{
    if(!s)
        return NULL;

    while(1)
    {
        if(!(*sz))
            return NULL;

        --(*sz);
        ++s;

        if(*s == '\0')
        {
            if(*sz == 0)
                return NULL;

            return s+1;
        }
    }
}