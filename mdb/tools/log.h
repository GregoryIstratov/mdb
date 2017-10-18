#pragma once

#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <pthread.h>
#include <mdb/config/config.h>
#include "compiler.h"


struct log_context
{
        int level;
        int verb;
        FILE* sink;
        pthread_mutex_t mtx;
};

struct log_context* __log_ctx;

enum
{
        LOGLEVEL_NONE   = 0,
        LOGLEVEL_WARN   = 1,
        LOGLEVEL_INFO   = 2,
        LOGLEVEL_DEBUG  = 3,
        LOGLEVEL_TRACE  = 4,
        LOGLEVEL_ALL    = 0xFFFFFF,

        LOG_ERROR       = 0,
        LOG_WARN        = 1,
        LOG_INFO        = 2,
        LOG_DEBUG       = 3,
        LOG_TRACE       = 4,
        LOG_ASSERT      = 5,

        LOG_NO_VERBOSE  = 0,
        LOG_VERBOSE1    = 1,
        LOG_VERBOSE2    = 2
};

#define LOG_ERROR(fmt, ...) \
        _log(__log_ctx, __FILE__, __LINE__, __func__, \
                LOG_ERROR, true,(fmt), ##__VA_ARGS__)

#define LOG_WARN(fmt, ...) \
        _log(__log_ctx, __FILE__, __LINE__, __func__, \
                LOG_WARN, true, (fmt), ##__VA_ARGS__)

#define LOG_VINFO(verb, fmt, ...) \
        _log_verbose(__log_ctx, __FILE__, __LINE__, __func__,\
                     LOG_INFO, (verb), true, (fmt), ##__VA_ARGS__)

#ifdef NDEBUG
#define LOG_DEBUG(fmt, ...)
#define LOG_TRACE(fmt, ...)
#else
#define LOG_DEBUG(fmt, ...) \
        _log(__log_ctx, __FILE__, __LINE__, __func__, \
                LOG_DEBUG, true,(fmt), ##__VA_ARGS__)


#define LOG_TRACE(fmt, ...) \
        _log(__log_ctx, __FILE__, __LINE__, __func__, \
                LOG_TRACE, true,(fmt), ##__VA_ARGS__)
#endif

#define LOG_SAY(fmt, ...) _log_say(__log_ctx, true, (fmt), ##__VA_ARGS__)
#define PARAM_INFO(label, fmt, ...) \
        _log_param(__log_ctx, true, (label), (fmt), ##__VA_ARGS__)




/* Create a new log context
 */
void log_init(int loglvl, int _verb_lvl, const char* filename);


/* Release all acquired resources by log system
 * and free the log context.
 */
void log_shutdown(void);

/* Set log context.
 * This function is for exporting log system to a kernel.
 * Kernel loader uses this function to join kernel logging system to the host.
 * You should never directly use it.
 */
void log_set_context(struct log_context* log);

void _log(struct log_context* log, const char* file, int line, const char* fun,
          int lvl, bool host, const char* fmt, ...)
__attribute__((format(printf, 7, 8)));

void _log_verbose(struct log_context* log, const char* file, int line,
                  const char* fun, int lvl, int verbose, bool host,
                  const char* fmt, ...)
__attribute__((format(printf, 8, 9)));

void _log_say(struct log_context* log, bool host, const char* fmt, ...)
__attribute__((format(printf, 3, 4)));

void _log_param(struct log_context* log, bool host,
                const char* label, const char* fmt, ...)
__attribute__((format(printf, 4, 5)));

void __log(struct log_context* log, const char* file, int line,
           const char* fun, int lvl, bool host, const char* fmt, va_list args);

void __log_user_info(struct log_context* log, bool host, const char* label,
                     const char* fmt, va_list args);