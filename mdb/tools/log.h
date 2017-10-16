#pragma once

#include <mdb/config/config.h>
#include "compiler.h"

enum
{
    LOGLEVEL_NONE   = 0,
    LOGLEVEL_WARN   = 1,
    LOGLEVEL_INFO   = 2,
    LOGLEVEL_DEBUG  = 3,
    LOGLEVEL_TRACE  = 4,
    LOGLEVEL_ALL    = 0xFFFFFF
};

enum
{
    LOG_ERROR   = 0,
    LOG_WARN    = 1,
    LOG_INFO    = 2,
    LOG_DEBUG   = 3,
    LOG_TRACE   = 4,
    LOG_ASSERT  = 5

};

enum
{
    LOG_NO_VERBOSE  = 0,
    LOG_VERBOSE1    = 1,
    LOG_VERBOSE2    = 2
};

#ifndef NDEBUG
#define LOG_ERROR(fmt, ...) _log(__FILE__, __LINE__, __func__, LOG_ERROR, (fmt),  ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) _log(__FILE__, __LINE__, __func__, LOG_WARN, (fmt) ,##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) _log(__FILE__, __LINE__, __func__, LOG_DEBUG, (fmt), ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) _log(__FILE__, __LINE__, __func__, LOG_INFO, (fmt), ##__VA_ARGS__)
#define LOG_VINFO(verb, fmt, ...) (_log_verbose(__FILE__, __LINE__, __func__, LOG_INFO, (verb), (fmt), ##__VA_ARGS__))
#define LOG_TRACE(fmt, ...) _log(__FILE__, __LINE__, __func__, LOG_TRACE, (fmt), ##__VA_ARGS__)
#define LOG_ASSERT(fmt, ...) _log(__FILE__, __LINE__, __func__, LOG_ASSERT, (fmt), ##__VA_ARGS__)
#else
#define LOG_ERROR(fmt, ...) (_log(__FILE__, __LINE__, __func__, LOG_ERROR, (fmt),  ##__VA_ARGS__))
#define LOG_WARN(fmt, ...) (_log(__FILE__, __LINE__, __func__, LOG_WARN, (fmt) ,##__VA_ARGS__))
#define LOG_DEBUG(fmt, ...) ((void)0)
#define LOG_INFO(fmt, ...) (_log(__FILE__, __LINE__, __func__, LOG_INFO, (fmt), ##__VA_ARGS__))
#define LOG_VINFO(verb, fmt, ...) (_log_verbose(__FILE__, __LINE__, __func__, LOG_INFO, (verb), (fmt), ##__VA_ARGS__))
#define LOG_TRACE(fmt, ...) ((void)0)
#define LOG_ASSERT(fmt, ...) ((void)0)
#endif

#define LOG_SAY(fmt, ...) _log_say((fmt), ##__VA_ARGS__)
#define PARAM_INFO(label, fmt, ...) _log_param((label), (fmt), ##__VA_ARGS__)


void log_init(int loglvl, int _verb_lvl, const char* filename);

void log_shutdown(void);

void _log(const char* file, int line, const char* fun, int lvl, const char* fmt, ...)
__attribute__((format(printf, 5, 6)));

void _log_verbose(const char* file, int line, const char* fun, int lvl, int verbose, const char* fmt, ...)
__attribute__((format(printf, 6, 7)));

__export_symbol
void _log_say(const char* fmt, ...)
__attribute__((format(printf, 1, 2)));;

__export_symbol
void _log_param(const char* label, const char* fmt, ...)
__attribute__((format(printf, 2, 3)));;;
