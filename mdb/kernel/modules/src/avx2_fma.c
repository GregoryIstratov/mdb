#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>
#include <stdalign.h>
#include <stdbool.h>

#include <kernel_module.h>

#if !defined(__AVX__)
#error "AVX is not enabled. Consider set gcc flags -mavx2"
#endif

#if !defined(__FMA__)
#error "FMA is not enabled. Consider set gcc flags -mfma"
#endif

static struct
{
    uint32_t bailout;
    uint32_t width;
    uint32_t height;
    float shift_x;
    float shift_y;
    float scale;
    float* f32surface;
    float width_r;
    float height_r;
    float aspect_ratio;
} mdb;

static const char* name = "Mandelbrot AVX2 FMA intrinsic kernel";
static const char* ver_maj = "1";
static const char* ver_min = "0";

int mdb_kernel_metadata_query(int query, char* buff, uint32_t buff_size)
{
    switch(query)
    {
        case MDB_KERNEL_META_NAME:
            return metadata_copy(name, buff, buff_size);

        case MDB_KERNEL_META_VER_MAJ:
            return metadata_copy(ver_maj, buff, buff_size);

        case MDB_KERNEL_META_VER_MIN:
            return metadata_copy(ver_min, buff, buff_size);

        default:
            return MDB_QUERY_NO_PARAM;
    }
}

int mdb_kernel_cpu_features(void)
{
    return CPU_FEATURE_AVX2 | CPU_FEATURE_FMA;
}

void mdb_kernel_init(void)
{

}

void mdb_kernel_shutdown(void)
{

}

void mdb_kernel_set_size(uint32_t width, uint32_t height)
{
    mdb.width = width;
    mdb.height = height;
    mdb.width_r = 1.0f / width;
    mdb.height_r = 1.0f / height;
    mdb.aspect_ratio = (float)width / height;
}

void mdb_kernel_set_scale(float scale)
{
    mdb.scale = scale;
}

void mdb_kernel_set_shift(float shift_x, float shift_y)
{
    mdb.shift_x = shift_x;
    mdb.shift_y = shift_y;
}


void mdb_kernel_set_bailout(uint32_t bailout)
{
    mdb.bailout = bailout;
}

void mdb_kernel_set_surface(float* buffer)
{
    mdb.f32surface = buffer;
}

void mdb_kernel_submit_changes(void)
{
}

void mdb_kernel_process_block(uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1)
{
    __m256 v_scale = _mm256_set1_ps(mdb.scale);
    __m256 v_shift_x = _mm256_set1_ps(mdb.shift_x);
    __m256 v_shift_y = _mm256_set1_ps(mdb.shift_y);
    __m256 v_center = _mm256_set1_ps(-0.5f);

    uint32_t bailout = mdb.bailout;

    __m256 v_width_r = _mm256_set1_ps(mdb.width_r);
    __m256 v_height_r = _mm256_set1_ps(mdb.height_r);
    __m256 v_wxh = _mm256_set1_ps(mdb.aspect_ratio);

    __m256 v_bound2 = _mm256_set1_ps(4);
    __m256 v_one = _mm256_set1_ps(1);

    for (uint32_t y = y0; y <= y1; ++y)
    {
        __m256 v_cy = _mm256_set1_ps(y);
        v_cy = _mm256_fmadd_ps(v_cy, v_height_r, v_center);
        v_cy = _mm256_fmadd_ps(v_cy, v_scale, v_shift_y);

        for (uint32_t x = x0; x < x1; x += 8)
        {

            __m256 v_cx = _mm256_set_ps(x + 7, x + 6, x + 5, x + 4,
                                        x + 3, x + 2, x + 1, x + 0);


            v_cx = _mm256_mul_ps(v_cx, v_width_r);
            v_cx = _mm256_fmadd_ps(v_cx, v_wxh, v_center);
            v_cx = _mm256_fmadd_ps(v_cx, v_scale, v_shift_x);


            __m256 v_zx = v_cx;
            __m256 v_zy = v_cy;

            uint32_t i = 0;
            __m256 v_i = _mm256_set1_ps(i);
            for (; i < bailout; ++i)
            {
                //zx1 = zx0 * zx0 - zy0 * zy0 + cx
                //zy1 = zx0 * zy0 + zx0 * zy0 + cy
                //-------------------------------
                //FMA tuned
                //zx1 = zx0 * zx0 - (zy0 * zy0 - cx)
                //zy1 = zx0 * zy0 + (zx0 * zy0 + cy)


                //zy0 * zy0 - cx
                __m256 v_zy2_cx = _mm256_fmsub_ps(v_zy, v_zy, v_cx);
                //zx0 * zx0 - zy2_cx
                __m256 v_zx1 = _mm256_fmsub_ps(v_zx, v_zx, v_zy2_cx);

                //zx0 * zy0 + cy
                __m256 v_zxzy_cy = _mm256_fmadd_ps(v_zx, v_zy, v_cy);
                //zx0 * zy0 + zxzy_cy
                __m256 v_zy1 = _mm256_fmadd_ps(v_zx, v_zy, v_zxzy_cy);



                //mag2 = zx * zx + zy * zy
                //mag2 = fma(zx,zx, mul(zy,zy))
                __m256 v_mag2 = _mm256_fmadd_ps(v_zx1, v_zx1, _mm256_mul_ps(v_zy1, v_zy1));

                __m256 bound_mask = _mm256_cmp_ps(v_mag2, v_bound2, _CMP_LT_OQ);

                int zero_mask = _mm256_movemask_ps(bound_mask);

                if ((!zero_mask))
                    break;

                __m256 add_mask = _mm256_and_ps(bound_mask, v_one);
                v_i = _mm256_add_ps(v_i, add_mask);

                v_zx = v_zx1;
                v_zy = v_zy1;

            }

            __m256 v_bailout = _mm256_set1_ps(bailout);
            __m256 bailout_mask = _mm256_cmp_ps(v_i, v_bailout, _CMP_NEQ_OQ);

            v_i = _mm256_and_ps(v_i, bailout_mask);

            v_i = _mm256_div_ps(v_i, v_bailout);

            alignas(32) float pixels[8];
            _mm256_store_ps(pixels, v_i);

            surface_set_pixels(mdb.f32surface, mdb.width, mdb.height, pixels, 8, x, y);
        }
    }
}