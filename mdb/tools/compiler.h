#pragma once

#define UNUSED_PARAM(x) ((void)(x))

# define likely(x)	__builtin_expect(!!(x), 1)
# define unlikely(x)	__builtin_expect(!!(x), 0)

#define __cold	__attribute__((__cold__))
#define __hot	__attribute__((__hot__))

#define __aligned(x) __attribute__((aligned(x)))

#define return_if(cond, val) if(cond) return (val);

#define __export_symbol __attribute__ ((visibility ("default")))