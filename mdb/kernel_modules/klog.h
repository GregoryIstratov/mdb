#pragma once

#include <stdbool.h>
#include <mdb/tools/log.h>

#undef LOG_ERROR
#undef LOG_WARN
#undef LOG_DEBUG
#undef LOG_TRACE
#undef LOG_SAY
#undef PARAM_INFO
#undef LOG_VINFO

#define KLOG_ERROR(fmt, ...) \
        _log(__log_ctx, __FILE__, __LINE__, __func__, \
                LOG_ERROR, false, (fmt), ##__VA_ARGS__)

#define KLOG_WARN(fmt, ...) \
        _log(__log_ctx, __FILE__, __LINE__, __func__, \
                LOG_WARN, false, (fmt), ##__VA_ARGS__)

#define KLOG_VINFO(verb, fmt, ...) \
        _log_verbose(__log_ctx, __FILE__, __LINE__, __func__,\
                     LOG_INFO, (verb), false, (fmt), ##__VA_ARGS__)

#ifdef NDEBUG
#define KLOG_DEBUG(fmt, ...)
#define KLOG_TRACE(fmt, ...)
#else
#define KLOG_DEBUG(fmt, ...) \
        _log(__log_ctx, __FILE__, __LINE__, __func__, \
                LOG_DEBUG, false, (fmt), ##__VA_ARGS__)


#define KLOG_TRACE(fmt, ...) \
        _log(__log_ctx, __FILE__, __LINE__, __func__, \
                LOG_TRACE, false, (fmt), ##__VA_ARGS__)
#endif

#define KLOG_SAY(fmt, ...) _log_say(__log_ctx, false, (fmt), ##__VA_ARGS__)
#define KPARAM_INFO(label, fmt, ...) \
        _log_param(__log_ctx, false, (label), (fmt), ##__VA_ARGS__)


__export_symbol
void register_log_context(struct log_context* log);