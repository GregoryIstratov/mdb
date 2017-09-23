#include <stdint.h>
#include <mdb/config/config.h>
#include "mdb_kernel.h"
#include "mdb_kernel_common.h"

#if defined(MDB_ENABLE_NATIVE_KERNEL)

MDB_KERNEL_DEFINE_PROCESS_BLOCK_FUN(mdb_kernel_process_block_native)

#endif