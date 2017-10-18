#pragma once

#include <stdlib.h>
#include <errno.h>
#include <mdb/tools/log.h>


#if defined(__unix__)
static inline
void* malloc_aligned(size_t size, size_t align)
{
        int res;
        void* mem;

        res = posix_memalign(&mem, align, size);
        if (unlikely(res))
        {
                if (ENOMEM == res)
                        LOG_ERROR("There was insufficient memory available "
                                          "to satisfy the request.");
                if (EINVAL == res)
                        LOG_ERROR("Alignment is not a power of two multiple "
                                          "of sizeof (void *).");

                return NULL;
        }

        return mem;
}

static inline
void free_aligned(void* ptr)
{
        free(ptr);
}

#elif (defined _WIN32 || defined __WIN32__) && ! defined __CYGWIN__

static inline
void* malloc_aligned(size_t size, size_t align)
{
        return _aligned_malloc(size, align);
}

static inline
void free_aligned(void* ptr)
{
        _aligned_free(ptr);
}

#endif