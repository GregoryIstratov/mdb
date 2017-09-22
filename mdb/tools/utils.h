#pragma once

#include <time.h>
#include <locale.h>
#include <stdio.h>

#define LOG_ERROR(msg,...) { fprintf(stderr, msg, ##__VA_ARGS__); fprintf(stderr, ": %m %s:%i\n", __FILE__, __LINE__); }
#define LOG_WARNING(msg, ...) LOG_ERROR(msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg,...) { fprintf(stdout, msg, ##__VA_ARGS__); fprintf(stdout, ": %m %s:%i\n", __FILE__, __LINE__); }
#define LOG_INFO(msg,...) { fprintf(stdout, msg, ##__VA_ARGS__); }

#define PARAM_INFO(label, fmt, ...) { fprintf(stdout, "%-20s: ", (label)); fprintf(stdout, (fmt), ##__VA_ARGS__); fprintf(stdout,"\n"); fflush(stdout); }
#define PARAM_DEBUG(label, fmt, ...) PARAM_INFO(label, fmt, ##__VA_ARGS__)




