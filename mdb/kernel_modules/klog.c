#include "klog.h"

void register_log_context(struct log_context* log)
{
        log_set_context(log);
}
