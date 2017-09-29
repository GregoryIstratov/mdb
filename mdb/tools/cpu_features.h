#pragma once

#include <stdlib.h>

#include <mdb/tools/bits/cpu_features.h>

/* Check for platform available cpu features
 * if everything is supported return 0
 * otherwise returns bitwise mask of unsupported features.
 */
int cpu_check_features(int mask);

/* Convert bitmask of cpu features to comma separated string */
int cpu_features_to_str(int mask, char* buff, size_t buff_sz);