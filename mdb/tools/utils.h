#pragma once

#include <time.h>
#include <locale.h>
#include <stdio.h>

#define LOG_ERRORNO(msg,...) { fprintf(stderr, msg, ##__VA_ARGS__); fprintf(stderr, ": %m - %s:%i\n", __FILE__, __LINE__); }
#define _LOG_MESSAGE(type, msg, ...) { fprintf(stdout, type); fprintf(stdout, msg, ##__VA_ARGS__); fprintf(stdout, " - %s:%i\n", __FILE__, __LINE__); }

#define LOG_ERROR(msg,...) { fprintf(stderr, "[ERROR] "); fprintf(stderr, msg, ##__VA_ARGS__); fprintf(stderr, " - %s:%i\n", __FILE__, __LINE__); }
#define LOG_WARNING(msg, ...) _LOG_MESSAGE("[WARNING] ", msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg,...) _LOG_MESSAGE("[DEBUG] ", msg, ##__VA_ARGS__)
#define LOG_INFO(msg,...) _LOG_MESSAGE("[INFO] ", msg, ##__VA_ARGS__)

#define PARAM_INFO(label, fmt, ...) { fprintf(stdout, "%-20s: ", (label)); fprintf(stdout, (fmt), ##__VA_ARGS__); fprintf(stdout,"\n"); fflush(stdout); }
#define PARAM_DEBUG(label, fmt, ...) PARAM_INFO(label, fmt, ##__VA_ARGS__)

#define UNUSED_PARAM(x) ((void)x)

# define likely(x)	__builtin_expect(!!(x), 1)
# define unlikely(x)	__builtin_expect(!!(x), 0)

#define __cold	__attribute__((__cold__))
#define __hot	__attribute__((__hot__))

#define CPU_CHECK_FEATURE(feature) __builtin_cpu_supports(feature)

int file_read_all(const char* filename, size_t* size, void** data);