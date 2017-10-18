#pragma once

#include <stdint.h>
#include <string.h>

#include <mdb/kernel/mdb_kernel_meta.h>
#include <mdb/kernel/mdb_kernel_event.h>
#include <mdb/tools/error_codes.h>
#include <mdb/tools/cpu_features.h>
#include <mdb/tools/compiler.h>
#include <mdb/surface/surface.h>

__hot
__export_symbol
void mdb_kernel_process_block(uint32_t x0, uint32_t x1,
                              uint32_t y0, uint32_t y1);
__export_symbol
int mdb_kernel_set_surface(struct surface* surf);
__export_symbol
int mdb_kernel_set_size(uint32_t width, uint32_t height);
__export_symbol
int mdb_kernel_shutdown(void);
__export_symbol
int mdb_kernel_init(void);
__export_symbol
int mdb_kernel_cpu_features(void);
__export_symbol
int mdb_kernel_metadata_query(int query, char* buff, uint32_t buff_size);
__export_symbol
int mdb_kernel_event_handler(int type, void* event);


static inline
int metadata_copy(const char* meta, char* dst, uint32_t dst_size)
{
        size_t meta_sz = strlen(meta) + 1;
        if(meta_sz > dst_size)
                return MDB_QUERY_BUFFER_OVERFLOW;

        memcpy(dst, meta, meta_sz);

        return MDB_QUERY_OK;
}
