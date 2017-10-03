#pragma once

#if 0 //!defined(__STDC_NO_ATOMICS__)

#include <stdatomic.h>

#define __atomic _Atomic

#undef atomic_store
#undef atomic_load
#undef atomic_fetch_add

#define atomic_store(PTR, VAL) \
        atomic_store_explicit(PTR, VAL, memory_order_release)

#define atomic_load(PTR) \
        atomic_load_explicit(PTR, memory_order_acquire)

#define atomic_compare_exchange(PTR, VAL, DES) \
        atomic_compare_exchange_weak_explicit(PTR, VAL, DES, memory_order_acq_rel, memory_order_acquire)

#define atomic_fetch_add(PTR, VAL) \
        atomic_fetch_add_explicit(PTR, VAL, memory_order_acq_rel);

#else

#define __atomic volatile

#define atomic_store(PTR, VAL) \
        __atomic_store_n(PTR, VAL, __ATOMIC_RELEASE)

#define atomic_load(PTR) \
        __atomic_load_n(PTR, __ATOMIC_ACQUIRE)

#define atomic_compare_exchange(PTR, VAL, DES) \
        __atomic_compare_exchange_n(PTR, VAL, DES, 1, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)

#define atomic_fetch_add(PTR, VAL) \
        __atomic_fetch_add(PTR, VAL, __ATOMIC_ACQ_REL);

#endif