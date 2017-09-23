#include <stdint.h>
#include "mdb_kernel.h"
#include "mdb_kernel_common.h"

/* Override default compiler options for generating common generic code
 * without any vector extension like sse,avx,fma, etc. and force it to use x87 math coprocessor instead of default sse
 */
__attribute__((target("arch=x86-64,tune=generic,no-mmx,no-sse,no-sse2,no-sse3,no-sse4,no-avx,no-avx2,no-fma,fpmath=387")))
MDB_KERNEL_DEFINE_PROCESS_BLOCK_FUN(mdb_kernel_process_block_generic)
