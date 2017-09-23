#pragma once

#define MDB_KERNEL_DEFINE_PROCESS_BLOCK_FUN(fun_name) void fun_name(struct _mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1) \
{\
    const mdb_float_t scale = mdb->scale;\
    const mdb_float_t shift_x = mdb->shift_x;\
    const mdb_float_t shift_y = mdb->shift_y;\
    static const mdb_float_t center = MDB_FLOAT_C(-0.5);\
\
    const mdb_float_t width_r = mdb->width_r;\
    const mdb_float_t height_r = mdb->height_r;\
    const mdb_float_t wxh = mdb->aspect_ratio;\
\
    const uint32_t bailout = mdb->bailout;\
    const mdb_float_t di = (mdb_float_t) 1 / bailout;\
\
    for (uint32_t y = y0; y <= y1; ++y)\
    {\
        mdb_float_t cy = (mdb_float_t) y * height_r;\
        cy += center;\
        cy *= scale;\
        cy += shift_y;\
\
        for (uint32_t x = x0; x <= x1; ++x)\
        {\
            mdb_float_t cx = (mdb_float_t) x * width_r * wxh;\
            cx += center;\
            cx *= scale;\
            cx += shift_x;\
\
            mdb_float_t zx = cx;\
            mdb_float_t zy = cy;\
\
            uint32_t i;\
            for (i = 0; i < bailout; ++i)\
            {\
                mdb_float_t zx2  = zx * zx;\
                mdb_float_t zy2  = zy * zy;\
                mdb_float_t zxzy = zx * zy;\
\
                zx = zx2 - zy2;\
                zx = zx  + cx;\
\
                zy = zxzy + zxzy;\
                zy = zy + cy;\
\
\
                zx2 = zx * zx;\
                zy2 = zy * zy;\
\
                mdb_float_t mag2 = zx2 + zy2;\
\
                if(mag2 > MDB_FLOAT_C(4.0))\
                    break;\
            }\
\
            if (i == bailout)\
                i = 0;\
\
            float norm_col = (float)(i * di);\
\
            mdb->f32surface[y * mdb->width + x] = norm_col;\
        }\
    }\
}
