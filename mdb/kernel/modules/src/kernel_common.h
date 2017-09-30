#pragma once

#define MDB_KERNEL_DEFINE_PROCESS_BLOCK_FUN(fun_name) void fun_name(uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1) \
{\
    const float scale = mdb.scale;\
    const float shift_x = mdb.shift_x;\
    const float shift_y = mdb.shift_y;\
    static const float center = -0.5f;\
\
    const float width_r = mdb.width_r;\
    const float height_r = mdb.height_r;\
    const float wxh = mdb.aspect_ratio;\
\
    const uint32_t bailout = mdb.bailout;\
    const float di = (float) 1 / bailout;\
\
    for (uint32_t y = y0; y <= y1; ++y)\
    {\
        float cy = (float) y * height_r;\
        cy += center;\
        cy *= scale;\
        cy += shift_y;\
\
        for (uint32_t x = x0; x <= x1; ++x)\
        {\
            float cx = (float) x * width_r * wxh;\
            cx += center;\
            cx *= scale;\
            cx += shift_x;\
\
            float zx = cx;\
            float zy = cy;\
\
            uint32_t i;\
            for (i = 0; i < bailout; ++i)\
            {\
                float zx2  = zx * zx;\
                float zy2  = zy * zy;\
                float zxzy = zx * zy;\
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
                float mag2 = zx2 + zy2;\
\
                if(mag2 > 4.0f)\
                    break;\
            }\
\
            if (i == bailout)\
                i = 0;\
\
            float norm_col = (float)(i * di);\
\
            surface_set_pixels(mdb.surf, x, y, 1, &norm_col); \
        }\
    }\
}
