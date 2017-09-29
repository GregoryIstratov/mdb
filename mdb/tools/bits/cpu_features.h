#pragma once

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
