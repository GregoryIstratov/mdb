#include "kernel_cpu.h"
#include <stdint.h>
#include <immintrin.h>
#include <stdalign.h>

typedef float mdb_float_t;

#define MDB_FLOAT_C(x) ((mdb_float_t)x)

typedef void(* mdb_block_fun)(mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);

struct _mdb_kernel
{
    int bailout;
    int width;
    int height;
    mdb_float_t shift_x;
    mdb_float_t shift_y;
    mdb_float_t scale;
    mdb_float_t width_r;
    mdb_float_t height_r;
    mdb_float_t aspect_ratio;
    mdb_block_fun block_fun;
    float* f32surface;
    int kernel_type;
};

static void mdb_kernel_process_block_avx_fma(mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);
static void mdb_kernel_process_block_avx(mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);
static void mdb_kernel_process_block_common(mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1);

int mdb_kernel_create(mdb_kernel** pmdb, int kernel_type, int width, int height, int bailout)
{
    *pmdb = calloc(1, sizeof(mdb_kernel));
    mdb_kernel* mdb = *pmdb;

    mdb->bailout = bailout;
    mdb->width = width;
    mdb->height = height;
    mdb->scale = MDB_FLOAT_C(0.00188964);
    mdb->shift_x = MDB_FLOAT_C(-1.347385054652062);
    mdb->shift_y = MDB_FLOAT_C(0.063483549665202);
    mdb->width_r = MDB_FLOAT_C(1.0) / width;
    mdb->height_r = MDB_FLOAT_C(1.0) / height;
    mdb->aspect_ratio = MDB_FLOAT_C(width) / MDB_FLOAT_C(height);
    mdb->kernel_type = kernel_type;

    switch (kernel_type)
    {
        case MDB_KERNEL_AVX_FMA:
            mdb->block_fun = &mdb_kernel_process_block_avx_fma;
            break;

        case MDB_KERNEL_AVX:
            mdb->block_fun = &mdb_kernel_process_block_avx;
            break;

        case MDB_KERNEL_GENERIC:
        case MDB_KERNEL_NATIVE:
            mdb->block_fun = &mdb_kernel_process_block_common;
            break;

        default:
            return -1;
    }

    return 0;
}

void mdb_kernel_set_surface(mdb_kernel* mdb, float* f32surface)
{
    mdb->f32surface = f32surface;
}

void mdb_kernel_set_size(mdb_kernel* mdb, uint32_t width, uint32_t height)
{
    mdb->width = width;
    mdb->height = height;
    mdb->width_r = MDB_FLOAT_C(1.0) / width;
    mdb->height_r = MDB_FLOAT_C(1.0) / height;
    mdb->aspect_ratio = MDB_FLOAT_C(width) / MDB_FLOAT_C(height);
}

void mdb_kernel_set_scale(mdb_kernel* mdb, float scale)
{
    mdb->scale = scale;
}

void mdb_kernel_set_shift(mdb_kernel* mdb, float shift_x, float shift_y)
{
    mdb->shift_x = shift_x;
    mdb->shift_y = shift_y;
}

void mdb_kernel_set_bailout(mdb_kernel* mdb, uint32_t bailout)
{
    mdb->bailout = bailout;
}

void mdb_kernel_submit_changes(mdb_kernel* mdb)
{
    ((void)mdb);
}

void mdb_kernel_process_block(mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1)
{
    mdb->block_fun(mdb, x0, x1, y0, y1);
}


static void mdb_kernel_process_block_avx_fma(mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1)
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

            alignas(32) float colors[8];
            _mm256_store_ps(colors, v_i);

            mdb->f32surface[y * mdb->width + x + 0] = colors[0];
            mdb->f32surface[y * mdb->width + x + 1] = colors[1];
            mdb->f32surface[y * mdb->width + x + 2] = colors[2];
            mdb->f32surface[y * mdb->width + x + 3] = colors[3];
            mdb->f32surface[y * mdb->width + x + 4] = colors[4];
            mdb->f32surface[y * mdb->width + x + 5] = colors[5];
            mdb->f32surface[y * mdb->width + x + 6] = colors[6];
            mdb->f32surface[y * mdb->width + x + 7] = colors[7];

        }
    }
}

static void mdb_kernel_process_block_avx(mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1)
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

            alignas(32) float colors[8];
            _mm256_store_ps(colors, v_i);

            mdb->f32surface[y * mdb->width + x + 0] = colors[0];
            mdb->f32surface[y * mdb->width + x + 1] = colors[1];
            mdb->f32surface[y * mdb->width + x + 2] = colors[2];
            mdb->f32surface[y * mdb->width + x + 3] = colors[3];
            mdb->f32surface[y * mdb->width + x + 4] = colors[4];
            mdb->f32surface[y * mdb->width + x + 5] = colors[5];
            mdb->f32surface[y * mdb->width + x + 6] = colors[6];
            mdb->f32surface[y * mdb->width + x + 7] = colors[7];

        }
    }
}

static void mdb_kernel_process_block_common(mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1)
{
    const mdb_float_t scale = mdb->scale;
    const mdb_float_t shift_x = mdb->shift_x;
    const mdb_float_t shift_y = mdb->shift_y;
    static const mdb_float_t center = MDB_FLOAT_C(-0.5);

    const mdb_float_t width_r = mdb->width_r;
    const mdb_float_t height_r = mdb->height_r;
    const mdb_float_t wxh = mdb->aspect_ratio;

    const uint32_t bailout = mdb->bailout;
    const mdb_float_t di = (mdb_float_t) 1 / bailout;

    for (uint32_t y = y0; y <= y1; ++y)
    {
        mdb_float_t cy = (mdb_float_t) y * height_r;
        cy += center;
        cy *= scale;
        cy += shift_y;

        for (uint32_t x = x0; x <= x1; ++x)
        {
            mdb_float_t cx = (mdb_float_t) x * width_r * wxh;
            cx += center;
            cx *= scale;
            cx += shift_x;

            mdb_float_t zx = cx;
            mdb_float_t zy = cy;

            uint32_t i;
            for (i = 0; i < bailout; ++i)
            {
                mdb_float_t zx2  = zx * zx;
                mdb_float_t zy2  = zy * zy;
                mdb_float_t zxzy = zx * zy;

                zx = zx2 - zy2;
                zx = zx  + cx;

                zy = zxzy + zxzy;
                zy = zy + cy;


                zx2 = zx * zx;
                zy2 = zy * zy;

                mdb_float_t mag2 = zx2 + zy2;

                if(mag2 > MDB_FLOAT_C(4.0))
                    break;
            }

            if (i == bailout)
                i = 0;

            float norm_col = (float)(i * di);

            mdb->f32surface[y * mdb->width + x] = norm_col;
        }
    }
}