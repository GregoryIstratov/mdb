#include <mandelbrot/bits/mdb_kernel_common.h>
#include <mandelbrot/bits/block_fun_common.h>


GLOBAL_VAR_INIT(const char*, name, "Mandelbrot generic kernel");
GLOBAL_VAR_INIT(const char*, ver_maj, "1");
GLOBAL_VAR_INIT(const char*, ver_min, "0");


int mdb_kernel_cpu_features(void)
{
    return 0;
}


/* Override default compiler options for generating common generic code
 * without any vector extension like sse,avx,fma, etc. and force it to use x87 math coprocessor instead of default sse
 */
__attribute__((target("arch=x86-64,tune=generic,no-mmx,no-sse,no-sse2,no-sse3,no-sse4,no-avx,no-avx2,no-fma,fpmath=387")))
MDB_KERNEL_DEFINE_PROCESS_BLOCK_FUN(mdb_kernel_process_block)
