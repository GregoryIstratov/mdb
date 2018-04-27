#include <mandelbrot/mdb_kernel_common.h>
#include <stdbool.h>
#include <math.h>
#include <kernel_modules/klog.h>


GLOBAL_VAR_INIT(const char*, name, "Mandelbrot native kernel");
GLOBAL_VAR_INIT(const char*, ver_maj, "1");
GLOBAL_VAR_INIT(const char*, ver_min, "0");

int mdb_kernel_cpu_features(void)
{
        return 0;
}


typedef float vec8f __attribute__((vector_size(32)));
typedef int vec8i __attribute__((vector_size(32)));


#define vec8f_set1_s(s) (vec8f){s, s, s, s, s, s, s, s}


#define vec8f_set_s(s0, s1, s2, s3, s4, s5, s6, s7) \
(vec8f){s0, s1, s2, s3, s4, s5, s6, s7}

#define vec8i_set1_s(s) (vec8i){s, s, s, s, s, s, s, s}


#define vec8i_set_s(s0, s1, s2, s3, s4, s5, s6, s7) \
(vec8i){s0, s1, s2, s3, s4, s5, s6, s7}


#define vec8i_el_at(v, i) (((int*)(v))[i])


#if defined(__AVX__)
static inline
int vec8i_test_zero(vec8i* v)
{
        return __builtin_ia32_movmskps256((vec8f)*v);
}
#else
static inline
int vec8i_test_zero(vec8i* v)
{
        int m = 0;
        int* vp = (int*)v;

        m |= vp[0];
        m |= vp[1];
        m |= vp[2];
        m |= vp[3];
        m |= vp[4];
        m |= vp[5];
        m |= vp[6];
        m |= vp[7];

        return m;
}
#endif

#if defined(__AVX__)
static inline
vec8f vec8i_cvt2f(vec8i* v)
{
        return __builtin_ia32_cvtdq2ps256(*v);
}
#else
static inline
vec8f vec8i_cvt2f(vec8i* v)
{
        int* vp = (int*)v;

        return vec8f_set_s(vp[0], vp[1], vp[2], vp[3],
                           vp[4], vp[5], vp[6], vp[7]);
}
#endif

#if defined(__AVX__)
static inline
void vec8f_store(float* dst, vec8f* v)
{
        __builtin_ia32_storeups256(dst, *v);
}
#else
static inline
void vec8f_store(float* dst, vec8f* v)
{
        float* vp = (float*)v;

        dst[0] = vp[0];
        dst[1] = vp[1];
        dst[2] = vp[2];
        dst[3] = vp[3];
        dst[4] = vp[4];
        dst[5] = vp[5];
        dst[6] = vp[6];
        dst[7] = vp[7];
}
#endif


void mdb_kernel_process_block(uint32_t x0, uint32_t x1,
                              uint32_t y0, uint32_t y1)
{
        __aligned(32) float pixels[8];

        uint32_t y, x;
        uint32_t i;

        const uint32_t bailout = mdb.bailout;

        vec8f scale, shift_x, shift_y, center, width_r, height_r, wxh;

        vec8f cy, cx, zx, zy, zx2, zy2, zxzy, mag2, bound, v_bailout_f, vif;
        vec8i add_mask, v_bailout_i;
        vec8i vi, mask, one;

        scale = vec8f_set1_s(mdb.scale);
        shift_x = vec8f_set1_s(mdb.shift_x);
        shift_y = vec8f_set1_s(mdb.shift_y);
        center = vec8f_set1_s(-0.5f);

        width_r = vec8f_set1_s(mdb.width_r);
        height_r = vec8f_set1_s(mdb.height_r);
        wxh = vec8f_set1_s(mdb.aspect_ratio);

        bound = vec8f_set1_s(4.0f);

        v_bailout_f = vec8f_set1_s(bailout);
        v_bailout_i = vec8i_set1_s(bailout);
        one = vec8i_set1_s(1);

        for (y = y0; y <= y1; ++y)
        {

                float fy = y;

                cy = vec8f_set1_s(fy);
                cy *=  height_r;
                cy += center;
                cy *= scale;
                cy += shift_y;

                for (x = x0; x <= x1; x += 8)
                {


                        cx = vec8f_set_s(x + 0.0f, x + 1.0f, x + 2.0f, x + 3.0f,
                                         x + 4.0f, x + 5.0f, x + 6.0f, x + 7.0f);

                        cx *= width_r;
                        cx *= wxh;
                        cx += center;
                        cx *= scale;
                        cx += shift_x;

                        vi = vec8i_set1_s(0);
                        zx = cx;
                        zy = cy;

                        for (i = 0; i < bailout; ++i)
                        {

                                int m;


                                zx2  = zx * zx;
                                zy2  = zy * zy;
                                zxzy = zx * zy;

                                zx = zx2 - zy2 + cx;
                                zy = zxzy + zxzy + cy;

                                zx2 = zx * zx;
                                zy2 = zy * zy;
                                mag2 = zx2 + zy2;

                                mask = mag2 < bound;

                                m = vec8i_test_zero(&mask);
                                if(m)
                                {
                                        add_mask = one & mask;
                                        vi += add_mask;
                                }
                                else
                                        break;
                        }


                        mask = vi != v_bailout_i;
                        vi &= mask;

                        vif = vec8i_cvt2f(&vi);
                        vif /= v_bailout_f;

                        vec8f_store(pixels, &vif);

                        surface_set_pixels(mdb.surf, x, y, 8, pixels);
                }
        }
}
