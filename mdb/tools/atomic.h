#pragma once

#define __atomic volatile

#define atomic_store(PTR, VAL) \
        __atomic_store_n(PTR, VAL, __ATOMIC_RELEASE)

#define atomic_load(PTR) \
        __atomic_load_n(PTR, __ATOMIC_ACQUIRE)

#define atomic_compare_exchange(PTR, VAL, DES) \
        __atomic_compare_exchange_n(PTR, VAL, DES, 1, \
                                    __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)

#define atomic_compare_exchange_strong(PTR, VAL, DES) \
        __atomic_compare_exchange_n(PTR, VAL, DES, 0, \
                                    __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)


#define atomic_fetch_add(PTR, VAL) \
        __atomic_fetch_add(PTR, VAL, __ATOMIC_ACQ_REL)


#define atomic_test_and_set(PTR) \
        __atomic_test_and_set(PTR, __ATOMIC_ACQ_REL)

#define atomic_clear(PTR) \
        __atomic_clear(PTR, __ATOMIC_RELEASE)

#define atomic_exchange(PTR, VAL) \
        __atomic_exchange_n(PTR, VAL, __ATOMIC_ACQ_REL)

