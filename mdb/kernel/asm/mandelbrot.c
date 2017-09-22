#include <stdlib.h>
#include <stdio.h>
#include <immintrin.h>

static void process_block(int x0, int x1, int y0, int y1, float* output)
{
    __m256 v_scale   = _mm256_set1_ps(0.00188964);
    __m256 v_shift_x = _mm256_set1_ps(-1.347385054652062);
    __m256 v_shift_y = _mm256_set1_ps(0.063483549665202);
    __m256 v_center  = _mm256_set1_ps(-0.5f);

    const int bailout = 256;

    __m256 v_width_r = _mm256_set1_ps((float)1 / 1024);
    __m256 v_height_r = _mm256_set1_ps((float)1 / 1024);
    __m256 v_wxh = _mm256_set1_ps(1);

    __m256 v_bound2 = _mm256_set1_ps(4);
    __m256 v_one = _mm256_set1_ps(1);

    for (int y = y0; y <= y1; ++y)
    {
        __m256 v_cy = _mm256_set1_ps(y);
        v_cy = _mm256_fmadd_ps(v_cy, v_height_r, v_center);
        v_cy = _mm256_fmadd_ps(v_cy, v_scale, v_shift_y);

        for (int x = x0; x < x1; x += 8)
        {

            __m256 v_cx = _mm256_set_ps(x + 7, x + 6, x + 5, x + 4,
                                        x + 3, x + 2, x + 1, x + 0);


            v_cx = _mm256_mul_ps(v_cx, v_width_r);
            v_cx = _mm256_fmadd_ps(v_cx, v_wxh, v_center);
            v_cx = _mm256_fmadd_ps(v_cx, v_scale, v_shift_x);


            __m256 v_zx = v_cx;
            __m256 v_zy = v_cy;

            int i = 0;
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

                if (!zero_mask)
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

            float* p = (float*)&(output[y*1024+x]);

            _mm256_storeu_ps(p, v_i);
        }
    }
}

/*

%define tmp0        r9
%define tmp1        r10
%define bailout     r13
%define i           r14
%define x_init      r15
%define output      r8
%define x           rdi
%define x1          rsi
%define y           rdx
%define y1          rcx
%define v_cy_l      xmm0
%define v_cy        ymm0
%define v_cx_l      xmm1
%define v_cx        ymm1
%define v_zy        ymm2
%define v_zx        ymm3
%define v_i         ymm4
%define v_zx1       ymm5
%define v_zy1       ymm6
%define v_zy2_cx    ymm7
%define v_zx1       ymm8
%define v_zy1       ymm9
%define v_zxzy_cy   ymm10
%define v_tmp4      ymm11
%define v_tmp3      ymm12
%define v_tmp2      ymm13
%define v_tmp1      ymm14
%define v_tmp0      ymm15
%define v_tmp0_l    xmm15

 */

int main()
{
    size_t buff_size = 1024 * 1024 * sizeof(float);
    float* buffer = malloc(buff_size);

    process_block(0, 1023, 0, 1023, buffer);

    FILE* f = fopen("mandelbrot.raw", "wb");

    fwrite(buffer, 1, buff_size, f);

    fclose(f);
}
