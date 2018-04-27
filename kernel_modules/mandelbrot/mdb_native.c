#include <mandelbrot/mdb_kernel_common.h>
#include <stdbool.h>
#include <math.h>


GLOBAL_VAR_INIT(const char*, name, "Mandelbrot native kernel");
GLOBAL_VAR_INIT(const char*, ver_maj, "1");
GLOBAL_VAR_INIT(const char*, ver_min, "0");

int mdb_kernel_cpu_features(void)
{
        return 0;
}

__aligned(32)
struct vec8
{
        union
        {
                int32_t i[8];

                float f[8];

                struct
                {
                        float f0, f1, f2, f3, f4, f5, f6, f7;
                };

                struct
                {
                        int32_t i0, i1, i2, i3, i4, i5, i6, i7;
                };
        };

};


#define VEC8F_OP_BI_V(a, op, b) \
do { \
        (a)->f0 op (b)->f0; \
        (a)->f1 op (b)->f1; \
        (a)->f2 op (b)->f2; \
        (a)->f3 op (b)->f3; \
        (a)->f4 op (b)->f4; \
        (a)->f5 op (b)->f5; \
        (a)->f6 op (b)->f6; \
        (a)->f7 op (b)->f7; \
} while(0)

#define VEC8F_OP_BI_S(a, op, b) \
do { \
        (a)->f0 op (b); \
        (a)->f1 op (b); \
        (a)->f2 op (b); \
        (a)->f3 op (b); \
        (a)->f4 op (b); \
        (a)->f5 op (b); \
        (a)->f6 op (b); \
        (a)->f7 op (b); \
        } while(0)

#define VEC8F_OP3_BI_V(res, a, op, b) \
do { \
        (res)->f0 = (a)->f0 op (b)->f0; \
        (res)->f1 = (a)->f1 op (b)->f1; \
        (res)->f2 = (a)->f2 op (b)->f2; \
        (res)->f3 = (a)->f3 op (b)->f3; \
        (res)->f4 = (a)->f4 op (b)->f4; \
        (res)->f5 = (a)->f5 op (b)->f5; \
        (res)->f6 = (a)->f6 op (b)->f6; \
        (res)->f7 = (a)->f7 op (b)->f7; \
} while(0)

#define VEC8F_OP3_BI_S(res, a, op, b) \
do { \
        (res)->f0 = (a)->f0 op (b); \
        (res)->f1 = (a)->f1 op (b); \
        (res)->f2 = (a)->f2 op (b); \
        (res)->f3 = (a)->f3 op (b); \
        (res)->f4 = (a)->f4 op (b); \
        (res)->f5 = (a)->f5 op (b); \
        (res)->f6 = (a)->f6 op (b); \
        (res)->f7 = (a)->f7 op (b); \
} while(0)

#define VEC8I_OP_BI_V(a, op, b) \
do { \
        (a)->i0 op (b)->i0; \
        (a)->i1 op (b)->i1; \
        (a)->i2 op (b)->i2; \
        (a)->i3 op (b)->i3; \
        (a)->i4 op (b)->i4; \
        (a)->i5 op (b)->i5; \
        (a)->i6 op (b)->i6; \
        (a)->i7 op (b)->i7; \
} while(0)

#define VEC8I_OP_BI_S(a, op, b) \
do { \
        (a)->i0 op (b); \
        (a)->i1 op (b); \
        (a)->i2 op (b); \
        (a)->i3 op (b); \
        (a)->i4 op (b); \
        (a)->i5 op (b); \
        (a)->i6 op (b); \
        (a)->i7 op (b); \
} while(0)

static inline
void vec8f_set1_s(struct vec8* v, float s)
{
        VEC8F_OP_BI_S(v, =, s);
}

static inline
void vec8i_set1_s(struct vec8* v, int32_t s)
{
        VEC8I_OP_BI_S(v, =, s);
}

static inline
void vec8f_set_s(struct vec8* v, float s0, float s1, float s2, float s3,
                 float s4, float s5, float s6, float s7)
{
        v->f0 = s0;
        v->f1 = s1;
        v->f2 = s2;
        v->f3 = s3;
        v->f4 = s4;
        v->f5 = s5;
        v->f6 = s6;
        v->f7 = s7;
}

static inline
void vec8f_cpy_v(struct vec8* a, const struct vec8* b)
{
        VEC8F_OP_BI_V(a, =, b);
}

static inline
void vec8f_mul2_v(struct vec8* a, const struct vec8* b)
{
        VEC8F_OP_BI_V(a, *=, b);
}

static inline
void vec8f_mul2_s(struct vec8* a, float b)
{
        VEC8F_OP_BI_S(a, *=, b);
}

static inline
void vec8f_div2_s(struct vec8* a, float b)
{
        VEC8F_OP_BI_S(a, /=, b);
}

static inline
void vec8i_div2_s(struct vec8* a, int32_t b)
{
        VEC8I_OP_BI_S(a, /=, b);
}

static inline
void vec8f_add2_v(struct vec8* a, const struct vec8* b)
{
        VEC8F_OP_BI_V(a, +=, b);
}

static inline
void vec8f_add2_s(struct vec8* a, float b)
{
        VEC8F_OP_BI_S(a, +=, b);
}

static inline
void vec8i_add2_mask_s(struct vec8* a, const struct vec8* mask,
                       int32_t i)
{
        int j;
        for(j = 0; j < 8; ++j)
                a->i[j] += mask->i[j] ? i : 0;
}

static inline
void vec8i_add2_s(struct vec8* a, int32_t b)
{
        VEC8I_OP_BI_S(a, +=, b);
}

static inline
void vec8f_sub2_v(struct vec8* a, const struct vec8* b)
{
        VEC8F_OP_BI_V(a, -=, b);
}

static inline
void vec8f_sub2_s(struct vec8* a, float b)
{
        VEC8F_OP_BI_S(a, -=, b);
}

static inline
void vec8i_and2_v(struct vec8* a, const struct vec8* b)
{
        VEC8I_OP_BI_V(a, &=, b);
}

static inline
void vec8f_mul3_v(struct vec8* res, const struct vec8* a, const struct vec8* b)
{
        VEC8F_OP3_BI_V(res, a, *, b);
}

static inline
void vec8f_mul3_s(struct vec8* res, const struct vec8* a, float b)
{
        VEC8F_OP3_BI_S(res, a, *, b);
}

static inline
void vec8f_div3_s(struct vec8* res, const struct vec8* a, float b)
{
        VEC8F_OP3_BI_S(res, a, /, b);
}

static inline
void vec8f_add3_v(struct vec8* res, const struct vec8* a, const struct vec8* b)
{
        VEC8F_OP3_BI_V(res, a, +, b);
}

static inline
void vec8f_add3_s(struct vec8* res, const struct vec8* a, float b)
{
        VEC8F_OP3_BI_S(res, a, +, b);
}

static inline
void vec8f_sub3_v(struct vec8* res, const struct vec8* a, const struct vec8* b)
{
        VEC8F_OP3_BI_V(res,  a, -, b);
}

static inline
void vec8f_sub3_s(struct vec8* res, const struct vec8* a, float b)
{
        VEC8F_OP3_BI_S(res, a, -, b);
}

static inline
bool vec8i_test_zf(const struct vec8* v)
{
        return !(v->i0
               | v->i1
               | v->i2
               | v->i3
               | v->i4
               | v->i5
               | v->i6
               | v->i7);
}

static inline
void vec8f_lt3_mask_s(struct vec8* m, const struct vec8* a, float b)
{

        m->i0 = -(a->f0 < b);
        m->i1 = -(a->f1 < b);
        m->i2 = -(a->f2 < b);
        m->i3 = -(a->f3 < b);
        m->i4 = -(a->f4 < b);
        m->i5 = -(a->f5 < b);
        m->i6 = -(a->f6 < b);
        m->i7 = -(a->f7 < b);
}

static inline
void vec8i_ne3_mask_s(struct vec8* m, const struct vec8* a, float b)
{
        m->i0 = -(a->i0 != b);
        m->i1 = -(a->i1 != b);
        m->i2 = -(a->i2 != b);
        m->i3 = -(a->i3 != b);
        m->i4 = -(a->i4 != b);
        m->i5 = -(a->i5 != b);
        m->i6 = -(a->i6 != b);
        m->i7 = -(a->i7 != b);
}

static inline
void vec8i_cvt2_vec8f(struct vec8* vf, const struct vec8* vi)
{
        vf->f0 = (float)vi->i0;
        vf->f1 = (float)vi->i1;
        vf->f2 = (float)vi->i2;
        vf->f3 = (float)vi->i3;
        vf->f4 = (float)vi->i4;
        vf->f5 = (float)vi->i5;
        vf->f6 = (float)vi->i6;
        vf->f7 = (float)vi->i7;
}

void mdb_kernel_process_block(uint32_t x0, uint32_t x1,
                              uint32_t y0, uint32_t y1)
{
        uint32_t y, x;
        uint32_t i;

        const uint32_t bailout = mdb.bailout;

        struct vec8 scale, shift_x, shift_y, center, width_r, height_r, wxh;

        struct vec8 cy, cx, zx, zy, zx2, zy2, zxzy, mag2, vi, mask;

        vec8f_set1_s(&scale, mdb.scale);
        vec8f_set1_s(&shift_x, mdb.shift_x);
        vec8f_set1_s(&shift_y, mdb.shift_y);
        vec8f_set1_s(&center, -0.5f);

        vec8f_set1_s(&width_r, mdb.width_r);
        vec8f_set1_s(&height_r, mdb.height_r);
        vec8f_set1_s(&wxh, mdb.aspect_ratio);

        for (y = y0; y <= y1; ++y)
        {

                vec8f_set1_s(&cy, y);
                vec8f_mul2_v(&cy, &height_r);
                vec8f_add2_v(&cy, &center);
                vec8f_mul2_v(&cy, &scale);
                vec8f_add2_v(&cy, &shift_y);

                for (x = x0; x <= x1; x += 8)
                {

                        vec8f_set_s(&cx, x + 0, x + 1, x + 2, x + 3,
                                         x + 4, x + 5, x + 6, x + 7);

                        vec8f_mul2_v(&cx, &width_r);
                        vec8f_mul2_v(&cx, &wxh);
                        vec8f_add2_v(&cx, &center);
                        vec8f_mul2_v(&cx, &scale);
                        vec8f_add2_v(&cx, &shift_x);


                        vec8i_set1_s(&vi, 0);
                        vec8f_cpy_v(&zx, &cx);
                        vec8f_cpy_v(&zy, &cy);
                        for (i = 0; i < bailout; ++i)
                        {

                                vec8f_mul3_v(&zx2, &zx, &zx);
                                vec8f_mul3_v(&zy2, &zy, &zy);
                                vec8f_mul3_v(&zxzy, &zx, &zy);

                                vec8f_sub3_v(&zx, &zx2, &zy2);
                                vec8f_add3_v(&zx, &zx, &cx);

                                vec8f_add3_v(&zy, &zxzy, &zxzy);
                                vec8f_add3_v(&zy, &zy, &cy);

                                vec8f_mul3_v(&zx2, &zx, &zx);
                                vec8f_mul3_v(&zy2, &zy, &zy);
                                vec8f_add3_v(&mag2, &zx2, &zy2);

                                vec8f_lt3_mask_s(&mask, &mag2, 4.0f);
                                if(!vec8i_test_zf(&mask))
                                {
                                        vec8i_add2_mask_s(&vi, &mask, 1);
                                }
                                else
                                        break;

                        }

                        vec8i_ne3_mask_s(&mask, &vi, bailout);
                        vec8i_and2_v(&vi, &mask);

                        vec8i_cvt2_vec8f(&vi, &vi);
                        vec8f_div2_s(&vi, bailout);

                        surface_set_pixels(mdb.surf, x, y, 8, vi.f);
                }
        }
}
