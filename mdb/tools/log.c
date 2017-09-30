#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

#include "log.h"
#include "compiler.h"

#define LOG_RED   "\x1B[31m"
#define LOG_GRN   "\x1B[32m"
#define LOG_YEL   "\x1B[33m"
#define LOG_BLU   "\x1B[34m"
#define LOG_MAG   "\x1B[35m"
#define LOG_CYN   "\x1B[36m"
#define LOG_WHT   "\x1B[37m"
#define LOG_RESET "\x1B[0m"

#ifdef LOG_ENABLE_MULTITHREADING
static pthread_mutex_t log_mtx;
#endif

static int loglevel = LOGLEVEL_DEBUG;
static FILE* log_sink = NULL;

void log_init(int loglvl, const char* filename) {
    loglevel = loglvl;
#ifdef LOG_ENABLE_MULTITHREADING
    pthread_mutex_init(&log_mtx, NULL);
#endif
    if(filename)
    {
        log_sink = fopen(filename, "a");
        if(!log_sink)
        {
            fprintf(stderr, "[log_init] Failed to open file '%s' for logging: %s\n", filename, strerror(errno));
            fprintf(stderr, "[log_init] Falling back to stdout log sink\n");

            log_sink = stdout;
        }
    }
    else
    {
        log_sink = stdout;
    }
}

void log_shutdown() {
    if (log_sink != stdout)
        fclose(log_sink);

#ifdef LOG_ENABLE_MULTITHREADING
    pthread_mutex_destroy(&log_mtx);
#endif
}

static const char* loglevel_s(int lvl) {
    switch (lvl) {
        case LOG_ERROR:
            return "ERR";
        case LOG_WARN:
            return "WRN";
        case LOG_DEBUG:
            return "DBG";
        case LOG_INFO:
            return "INF";
        case LOG_TRACE:
            return "TRC";
        case LOG_ASSERT:
            return "ASSERTION FAILED";
        default:
            return "UNKNOWN";
    }
}

static const char* log_color(int lvl) {
    switch (lvl) {
        case LOG_ERROR:
            return LOG_RED;
        case LOG_DEBUG:
            return LOG_CYN;
        case LOG_INFO:
            return LOG_GRN;
        case LOG_TRACE:
            return LOG_WHT;
        case LOG_WARN:
            return LOG_YEL;
        case LOG_ASSERT:
            return LOG_YEL;
        default:
            return LOG_RESET;
    }
}

void _log(const char* file, int line, const char* fun, int lvl, const char* fmt, ...) {
    if (lvl <= loglevel) {

#ifdef LOG_ENABLE_MULTITHREADING
        pthread_mutex_lock(&log_mtx);
#endif

#if defined LOG_SHOW_TIME || defined LOG_SHOW_DATE
        time_t t;
        struct tm _tml;
        struct tm* tml;
        if (time(&t) == (time_t)-1) {
            fprintf(stderr, "[_log] time returns failure");
            return;
        }

        localtime_r(&t, &_tml);
        tml = &_tml;

#endif

        fprintf(log_sink, "%s", log_color(lvl));
#ifdef LOG_SHOW_TIME
        fprintf(log_sink, "[%02d:%02d:%02d]", tml->tm_hour, tml->tm_min, tml->tm_sec);
#endif
#ifdef LOG_SHOW_DATE
        fprintf(log_sink, "[%02d/%02d/%d]", tml->tm_mday, tml->tm_mon + 1, tml->tm_year - 100);
#endif
#ifdef LOG_SHOW_THREAD
        pid_t tid = (pid_t)syscall(__NR_gettid);
        fprintf(log_sink, "[0x%08X]", tid);
#endif

        fprintf(log_sink, "[%s][%s]: ", loglevel_s(lvl), fun);

        va_list args;
        va_start(args, fmt);
        vfprintf(log_sink, fmt, args);
        va_end(args);

        fprintf(log_sink, "%s", LOG_RESET);

#ifdef LOG_SHOW_PATH
        fprintf(log_sink, " - %s:%i", file, line);
#else
        UNUSED_PARAM(file);
        UNUSED_PARAM(line);
#endif
        fprintf(log_sink, "\n");
        fflush(log_sink);

#ifdef LOG_ENABLE_MULTITHREADING
        pthread_mutex_unlock(&log_mtx);
#endif

    }
}

void _log_say(const char* fmt, ...)
{
#ifdef LOG_ENABLE_MULTITHREADING
    pthread_mutex_lock(&log_mtx);
#endif

    va_list args;
    va_start(args, fmt);
    vfprintf(log_sink, fmt, args);
    va_end(args);

    fprintf(log_sink, "\n");
    fflush(log_sink);

#ifdef LOG_ENABLE_MULTITHREADING
    pthread_mutex_unlock(&log_mtx);
#endif
}


void _log_param(const char* label, const char* fmt, ...)
{
#ifdef LOG_ENABLE_MULTITHREADING
    pthread_mutex_lock(&log_mtx);
#endif

    fprintf(log_sink, "%-20s: ", label);

    va_list args;
    va_start(args, fmt);
    vfprintf(log_sink, fmt, args);
    va_end(args);

    fprintf(log_sink, "\n");
    fflush(log_sink);

#ifdef LOG_ENABLE_MULTITHREADING
    pthread_mutex_unlock(&log_mtx);
#endif
}