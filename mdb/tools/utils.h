#pragma once

#define UNUSED_PARAM(x) ((void)(x))

# define likely(x)	__builtin_expect(!!(x), 1)
# define unlikely(x)	__builtin_expect(!!(x), 0)

#define __cold	__attribute__((__cold__))
#define __hot	__attribute__((__hot__))

#define CPU_CHECK_FEATURE(feature) __builtin_cpu_supports(feature)
