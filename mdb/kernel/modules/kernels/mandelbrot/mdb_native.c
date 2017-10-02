#include "bits/mdb_kernel_common.h"
#include "bits/block_fun_common.h"


GLOBAL_VAR_INIT(const char*, name, "Mandelbrot native kernel");
GLOBAL_VAR_INIT(const char*, ver_maj, "1");
GLOBAL_VAR_INIT(const char*, ver_min, "0");

int mdb_kernel_cpu_features(void)
{
    return 0;
}


MDB_KERNEL_DEFINE_PROCESS_BLOCK_FUN(mdb_kernel_process_block)
