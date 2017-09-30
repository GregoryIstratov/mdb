#pragma once

#include <stdint.h>
#include <string.h>

#include <mdb/kernel/bits/mdb_kernel_meta.h>
#include <mdb/tools/bits/cpu_features.h>
#include <mdb/tools/compiler.h>
#include <mdb/surface/surface.h>

__always_inline static int metadata_copy(const char* meta, char* dst, uint32_t dst_size)
{
    size_t meta_sz = strlen(meta) + 1;
    if(meta_sz > dst_size)
        return MDB_QUERY_BUFFER_OVERFLOW;

    memcpy(dst, meta, meta_sz);

    return MDB_QUERY_OK;
}