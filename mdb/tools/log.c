#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <stdbool.h>

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

static int loglevel = LOGLEVEL_DEBUG;
static int verb_lvl = LOG_NO_VERBOSE;
static FILE* log_sink = NULL;
static pthread_mutex_t log_mtx;

void log_init(int loglvl, int _verb_lvl, const char* filename)
{
        loglevel = loglvl;
        verb_lvl = _verb_lvl;

        if(IS_ENABLED(CONFIG_LOG_MULTITHREADING))
                pthread_mutex_init(&log_mtx, NULL);

        if(!filename)
        {
                log_sink = stdout;
                return;
        }

        log_sink = fopen(filename, "a");
        if(!log_sink)
        {
                fprintf(stderr,
                        "[log_init] Failed to open file '%s' for logging: %s\n",
                        filename, strerror(errno));

                fprintf(stderr, "[log_init] Falling back to stdout log sink\n");

                log_sink = stdout;
        }
}

void log_shutdown()
{
        if (log_sink != stdout)
                fclose(log_sink);

        if(IS_ENABLED(CONFIG_LOG_MULTITHREADING))
                pthread_mutex_destroy(&log_mtx);
}

static inline
const char* loglevel_s(int lvl)
{
        switch (lvl)
        {
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

static inline
const char* log_color(int lvl)
{
        switch (lvl)
        {
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


static inline
void _log_append_datetime(FILE* sink)
{
        time_t t;
        struct tm _tml;
        struct tm* tml;

        if (time(&t) == (time_t)-1) {
                fprintf(stderr, "[_log] time returns failure");
                return;
        }

        localtime_r(&t, &_tml);
        tml = &_tml;

        if(IS_ENABLED(CONFIG_LOG_TIME))
                fprintf(sink, "[%02d:%02d:%02d]",
                        tml->tm_hour, tml->tm_min, tml->tm_sec);

        if(IS_ENABLED(CONFIG_LOG_DATE))
                fprintf(sink, "[%02d/%02d/%d]",
                        tml->tm_mday, tml->tm_mon + 1, tml->tm_year - 100);

}

static inline
void _log_append_tid(FILE* sink)
{
        pid_t tid = (pid_t)syscall(__NR_gettid);
        fprintf(sink, "[0x%08X]", tid);
}

static inline
void __log(const char* file, int line, const char* fun, int lvl, bool host,
           const char* fmt, va_list args)
{
        if (lvl > loglevel)
                return;

        if(IS_ENABLED(CONFIG_LOG_MULTITHREADING))
                pthread_mutex_lock(&log_mtx);


        if(IS_ENABLED(CONFIG_LOG_COLOR))
                fprintf(log_sink, "%s", log_color(lvl));

        if(IS_ENABLED(CONFIG_LOG_TIME) || IS_ENABLED(CONFIG_LOG_DATE))
                _log_append_datetime(log_sink);

        if(IS_ENABLED(CONFIG_LOG_THREAD))
                _log_append_tid(log_sink);

        fprintf(log_sink, host ? "[HOST]" : "[KERN]");
        fprintf(log_sink, "[%s]", loglevel_s(lvl));

        if(IS_ENABLED(CONFIG_LOG_FUNC))
                fprintf(log_sink, "[%s]: ", fun);
        else
                fprintf(log_sink, ": ");

        vfprintf(log_sink, fmt, args);

        if(IS_ENABLED(CONFIG_LOG_COLOR))
                fprintf(log_sink, "%s", LOG_RESET);

        if(IS_ENABLED(CONFIG_LOG_PATH))
                fprintf(log_sink, " - %s:%i", file, line);

        fprintf(log_sink, "\n");
        fflush(log_sink);

        if(IS_ENABLED(CONFIG_LOG_MULTITHREADING))
                pthread_mutex_unlock(&log_mtx);

}

void _log(const char* file, int line, const char* fun,
          int lvl, bool host, const char* fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        __log(file, line, fun, lvl, host, fmt, args);
        va_end(args);
}

void _log_verbose(const char* file, int line, const char* fun,
                  int lvl, int verbose, bool host, const char* fmt, ...)
{
        va_list args;

        if(verb_lvl < verbose)
                return;

        va_start(args, fmt);
        __log(file, line, fun, lvl, host, fmt, args);
        va_end(args);

}

static inline
void __log_user_info(bool host, const char* label,
                     const char* fmt, va_list args)
{

        if(IS_ENABLED(CONFIG_LOG_MULTITHREADING))
                pthread_mutex_lock(&log_mtx);

        if(IS_ENABLED(CONFIG_LOG_COLOR))
                fprintf(log_sink, "%s", log_color(LOG_INFO));

        if(IS_ENABLED(CONFIG_LOG_TIME) || IS_ENABLED(CONFIG_LOG_DATE))
                _log_append_datetime(log_sink);

        fprintf(log_sink, host ? "[HOST]" : "[KERN]");
        fprintf(log_sink, "[%s]: ", loglevel_s(LOG_INFO));

        if(IS_ENABLED(CONFIG_LOG_COLOR))
                fprintf(log_sink, "%s", LOG_RESET);

        if(label)
                fprintf(log_sink, "%-20s: ", label);

        vfprintf(log_sink, fmt, args);

        fprintf(log_sink, "\n");
        fflush(log_sink);

        if(IS_ENABLED(CONFIG_LOG_MULTITHREADING))
                pthread_mutex_unlock(&log_mtx);
}

void _log_say(bool host, const char* fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        __log_user_info(host, NULL, fmt, args);
        va_end(args);
}


void _log_param(bool host, const char* label, const char* fmt, ...)
{
        va_list args;

        va_start(args, fmt);
        __log_user_info(host, label, fmt, args);
        va_end(args);
}