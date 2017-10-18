#include "nproc.h"
#include <mdb/tools/log.h>



#if defined(__unix__)
#include <sys/sysinfo.h>

int nproc_active(void)
{
        int n = get_nprocs();
        if(n > 0)
                return n;

        LOG_ERROR("Failed to get number of processors. Fallback to 1");
        return 1;
}

#endif

#if (defined _WIN32 || defined __WIN32__) && ! defined __CYGWIN__
# define WIN32_LEAN_AND_MEAN
# include <windows.h>

int nproc_active(void)
{
        SYSTEM_INFO system_info;
        GetSystemInfo (&system_info);
        if (0 < system_info.dwNumberOfProcessors)
                return system_info.dwNumberOfProcessors;

        LOG_ERROR("Failed to get number of processors. Fallback to 1");
        return 1;
}
#endif