#include "cpu_features.h"
#include <string.h>
#include <mdb/tools/compiler.h>



#define __cpu_probe_feature(mask, feature, name) \
if((mask) & (feature)) \
{ \
    if(__builtin_cpu_supports(name)) \
        (mask) ^= (feature); \
}

int cpu_check_features(int mask)
{
    return_if(!mask, mask);

    __cpu_probe_feature(mask, CPU_FEATURE_MMX, "mmx");

    return_if(!mask, mask);

    __cpu_probe_feature(mask, CPU_FEATURE_SSE, "sse");

    return_if(!mask, mask);

    __cpu_probe_feature(mask, CPU_FEATURE_SSE2, "sse2");

    return_if(!mask, mask);

    __cpu_probe_feature(mask, CPU_FEATURE_SSE3, "sse3");

    return_if(!mask, mask);

    __cpu_probe_feature(mask, CPU_FEATURE_SSSE3, "ssse3");

    return_if(!mask, mask);

    __cpu_probe_feature(mask, CPU_FEATURE_SSE4_1, "sse4.1");

    return_if(!mask, mask);

    __cpu_probe_feature(mask, CPU_FEATURE_SSE4_2, "sse4.2");

    return_if(!mask, mask);

    __cpu_probe_feature(mask, CPU_FEATURE_AVX, "avx");

    return_if(!mask, mask);

    __cpu_probe_feature(mask, CPU_FEATURE_AVX2, "avx2");

    return_if(!mask, mask);

    /* GCC 4.8.xx does not recognize 'fma' flag so
     * assume fma3 is available if avx2 is   */
    __cpu_probe_feature(mask, CPU_FEATURE_FMA, "avx2");

    return mask;

}

static int str_append(char** pdst, size_t* pdst_sz, const char* src)
{
    size_t src_len = strlen(src);
    if(src_len + 1 <= *pdst_sz)
    {
        memcpy(*pdst, src, src_len);
        *pdst = (*pdst) + src_len;
        *(*pdst) = ',';
        ++(*pdst);
        (*pdst_sz) -= src_len + 1;
        return 0;
    }

    return -1;
}



int cpu_features_to_str(int mask, char* buff, size_t buff_sz)
{
#define __cpu_strcpy_feature(feature, name) \
if((mask) & (feature)) \
{ \
    if(str_append(&buff, &buff_sz, (name))) \
        return -1; \
    \
    (mask) ^= (feature); \
}

    if(!mask)
    {
        buff[0] = '\0';
        return 0;
    }

    __cpu_strcpy_feature(CPU_FEATURE_MMX, "mmx");

    if(!mask) goto exit;

    __cpu_strcpy_feature(CPU_FEATURE_SSE, "sse");

    if(!mask) goto exit;

    __cpu_strcpy_feature(CPU_FEATURE_SSE2, "sse2");

    if(!mask) goto exit;

    __cpu_strcpy_feature(CPU_FEATURE_SSE3, "sse3");

    if(!mask) goto exit;

    __cpu_strcpy_feature(CPU_FEATURE_SSSE3, "ssse3");

    if(!mask) goto exit;

    __cpu_strcpy_feature(CPU_FEATURE_SSE4_1, "sse4.1");

    if(!mask) goto exit;

    __cpu_strcpy_feature(CPU_FEATURE_SSE4_2, "sse4.2");

    if(!mask) goto exit;

    __cpu_strcpy_feature(CPU_FEATURE_AVX, "avx");

    return_if(!mask, 0);

    __cpu_strcpy_feature(CPU_FEATURE_AVX2, "avx2");

    return_if(!mask, 0);

    __cpu_strcpy_feature(CPU_FEATURE_FMA, "fma");

    exit:
    *(buff-1) = '\0';
    return 0;

#undef __cpu_strcpy_feature
}