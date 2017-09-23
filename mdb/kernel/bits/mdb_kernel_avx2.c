#include <stdint.h>
#include <immintrin.h>
#include <stdalign.h>

#include <mdb/config/config.h>

#include "mdb_kernel.h"
#include "surface.h"

#if defined(MDB_ENABLE_AVX2_KERNEL)

#if !defined(__AVX__)
#error "AVX is not enabled. Consider set gcc flags -mavx2"
#endif

void mdb_kernel_process_block_avx2(struct _mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1)
{
    __m256 v_scale = _mm256_set1_ps(mdb->scale);
    __m256 v_shift_x = _mm256_set1_ps(mdb->shift_x);
    __m256 v_shift_y = _mm256_set1_ps(mdb->shift_y);
    __m256 v_center = _mm256_set1_ps(-0.5f);

    uint32_t bailout = mdb->bailout;

    __m256 v_width_r = _mm256_set1_ps(mdb->width_r);
    __m256 v_height_r = _mm256_set1_ps(mdb->height_r);
    __m256 v_wxh = _mm256_set1_ps(mdb->aspect_ratio);

    __m256 v_bound2 = _mm256_set1_ps(4);
    __m256 v_one = _mm256_set1_ps(1);

    for (uint32_t y = y0; y <= y1; ++y)
    {
        __m256 v_cy = _mm256_set1_ps(y);
        v_cy = _mm256_mul_ps(v_cy, v_height_r);
        v_cy = _mm256_add_ps(v_cy, v_center);
        v_cy = _mm256_mul_ps(v_cy, v_scale);
        v_cy = _mm256_add_ps(v_cy, v_shift_y);

        for (uint32_t x = x0; x < x1; x += 8)
        {

            __m256 v_cx = _mm256_set_ps(x + 7, x + 6, x + 5, x + 4,
                                        x + 3, x + 2, x + 1, x + 0);


            v_cx = _mm256_mul_ps(v_cx, v_width_r);
            v_cx = _mm256_mul_ps(v_cx, v_wxh);
            v_cx = _mm256_add_ps(v_cx, v_center);
            v_cx = _mm256_mul_ps(v_cx, v_scale);
            v_cx = _mm256_add_ps(v_cx, v_shift_x);


            __m256 v_zx = v_cx;
            __m256 v_zy = v_cy;

            uint32_t i = 0;
            __m256 v_i = _mm256_set1_ps(i);
            for (; i < bailout; ++i)
            {

                __m256 v_zx2 = _mm256_mul_ps(v_zx, v_zx);
                __m256 v_zy2 = _mm256_mul_ps(v_zy, v_zy);
                __m256 v_zxzy = _mm256_mul_ps(v_zx, v_zy);

                v_zx = _mm256_sub_ps(v_zx2, v_zy2);
                v_zx = _mm256_add_ps(v_zx, v_cx);

                v_zy = _mm256_add_ps(v_zxzy, v_zxzy);
                v_zy = _mm256_add_ps(v_zy, v_cy);


                v_zx2 = _mm256_mul_ps(v_zx, v_zx);
                v_zy2 = _mm256_mul_ps(v_zy, v_zy);
                __m256 v_mag2 = _mm256_add_ps(v_zx2, v_zy2);

                __m256 bound_mask = _mm256_cmp_ps(v_mag2, v_bound2, _CMP_LT_OQ);

                int zero_mask = _mm256_movemask_ps(bound_mask);

                if (!zero_mask)
                    break;

                __m256 add_mask = _mm256_and_ps(bound_mask, v_one);
                v_i = _mm256_add_ps(v_i, add_mask);


            }

            __m256 v_bailout = _mm256_set1_ps(bailout);
            __m256 bailout_mask = _mm256_cmp_ps(v_i, v_bailout, _CMP_NEQ_OQ);

            v_i = _mm256_and_ps(v_i, bailout_mask);

            v_i = _mm256_div_ps(v_i, v_bailout);

            alignas(32) float pixels[8];
            _mm256_store_ps(pixels, v_i);

            surface_set_pixels(mdb->f32surface, mdb->width, mdb->height, pixels, 8, x, y);

        }
    }
}

#endif