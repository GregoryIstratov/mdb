#pragma once

#include <stdint.h>
#include <string.h>

#include <mdb/kernel/bits/mdb_kernel_meta.h>
#include <mdb/kernel/bits/mdb_kernel_event.h>
#include <mdb/kernel/bits/mdb_kernel_error.h>
#include <mdb/tools/cpu_features.h>
#include <mdb/tools/compiler.h>
#include <mdb/surface/surface.h>

__export_sym void mdb_kernel_process_block(uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);
__export_sym void mdb_kernel_set_surface(surface* surf);
__export_sym void mdb_kernel_set_size(uint32_t width, uint32_t height);
__export_sym void mdb_kernel_shutdown(void);
__export_sym void mdb_kernel_init(void);
__export_sym int  mdb_kernel_cpu_features(void);
__export_sym int mdb_kernel_metadata_query(int query, char* buff, uint32_t buff_size);
__export_sym int mdb_kernel_event_handler(int type, void* event);


__always_inline static int metadata_copy(const char* meta, char* dst, uint32_t dst_size)
{
    size_t meta_sz = strlen(meta) + 1;
    if(meta_sz > dst_size)
        return MDB_QUERY_BUFFER_OVERFLOW;

    memcpy(dst, meta, meta_sz);

    return MDB_QUERY_OK;
}