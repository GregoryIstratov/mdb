#pragma once

#include <stdint.h>
#include <string.h>

#include <mdb/kernel/mdb_kernel_meta.h>
#include <mdb/kernel/mdb_kernel_event.h>
#include <mdb/tools/error_codes.h>
#include <mdb/tools/cpu_features.h>
#include <mdb/tools/compiler.h>
#include <mdb/surface/surface.h>
#include <mdb/tools/log.h>

#undef LOG_ERROR
#undef LOG_WARN
#undef LOG_DEBUG
#undef LOG_TRACE
#undef LOG_SAY
#undef PARAM_INFO
#undef LOG_VINFO

#define KLOG_ERROR(fmt, ...) \
        _log(__FILE__, __LINE__, __func__, LOG_ERROR, false,(fmt), ##__VA_ARGS__)

#define KLOG_WARN(fmt, ...) \
        _log(__FILE__, __LINE__, __func__, LOG_WARN, false, (fmt), ##__VA_ARGS__)

#define KLOG_VINFO(verb, fmt, ...) \
        _log_verbose(__FILE__, __LINE__, __func__,\
                     LOG_INFO, (verb), false, (fmt), ##__VA_ARGS__)

#ifdef NDEBUG
#define KLOG_DEBUG(fmt, ...)
#define KLOG_TRACE(fmt, ...)
#else
#define KLOG_DEBUG(fmt, ...) \
        _log(__FILE__, __LINE__, __func__, LOG_DEBUG, false,(fmt), ##__VA_ARGS__)


#define KLOG_TRACE(fmt, ...) \
        _log(__FILE__, __LINE__, __func__, LOG_TRACE, false,(fmt), ##__VA_ARGS__)
#endif

#define KLOG_SAY(fmt, ...) _log_say(false, (fmt), ##__VA_ARGS__)
#define KPARAM_INFO(label, fmt, ...) \
        _log_param(false, (label), (fmt), ##__VA_ARGS__)

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
