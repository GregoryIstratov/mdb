#pragma once

#include <stdbool.h>
#include <mdb/config/config.h>
#include "compiler.h"


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
        _log(__FILE__, __LINE__, __func__, LOG_ERROR, true,(fmt), ##__VA_ARGS__)

#define LOG_WARN(fmt, ...) \
        _log(__FILE__, __LINE__, __func__, LOG_WARN, true, (fmt), ##__VA_ARGS__)

#define LOG_VINFO(verb, fmt, ...) \
        _log_verbose(__FILE__, __LINE__, __func__,\
                     LOG_INFO, (verb), true, (fmt), ##__VA_ARGS__)

#ifdef NDEBUG
#define LOG_DEBUG(fmt, ...)
#define LOG_TRACE(fmt, ...)
#else
#define LOG_DEBUG(fmt, ...) \
        _log(__FILE__, __LINE__, __func__, LOG_DEBUG, true,(fmt), ##__VA_ARGS__)


#define LOG_TRACE(fmt, ...) \
        _log(__FILE__, __LINE__, __func__, LOG_TRACE, true,(fmt), ##__VA_ARGS__)
#endif

#define LOG_SAY(fmt, ...) _log_say(true, (fmt), ##__VA_ARGS__)
#define PARAM_INFO(label, fmt, ...) \
        _log_param(true, (label), (fmt), ##__VA_ARGS__)


void log_init(int loglvl, int _verb_lvl, const char* filename);

void log_shutdown(void);

__export_symbol
void _log(const char* file, int line, const char* fun,
          int lvl, bool host, const char* fmt, ...)
__attribute__((format(printf, 6, 7)));

__export_symbol
void _log_verbose(const char* file, int line, const char* fun,
                  int lvl, int verbose, bool host, const char* fmt, ...)
__attribute__((format(printf, 7, 8)));

__export_symbol
void _log_say(bool host, const char* fmt, ...)
__attribute__((format(printf, 2, 3)));

__export_symbol
void _log_param(bool host, const char* label, const char* fmt, ...)
__attribute__((format(printf, 3, 4)));
