#pragma once

#include <stdlib.h>

enum
{
        CPU_FEATURE_MMX     = 1,
        CPU_FEATURE_SSE     = 1<<1,
        CPU_FEATURE_SSE2    = 1<<2,
        CPU_FEATURE_SSE3    = 1<<3,
        CPU_FEATURE_SSSE3   = 1<<4,
        CPU_FEATURE_SSE4_1  = 1<<5,
        CPU_FEATURE_SSE4_2  = 1<<6,
        CPU_FEATURE_AVX     = 1<<7,
        CPU_FEATURE_AVX2    = 1<<8,
        CPU_FEATURE_FMA     = 1<<9
};

/* Check for available cpu features
 * if everything is supported returns 0
 * otherwise returns a bitwise mask of unsupported features.
 */
int cpu_check_features(int mask);

/* Convert a bitmask of the CPU features to a comma separated string */
int cpu_features_to_str(int mask, char* buff, size_t buff_sz);
