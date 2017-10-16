#include <mdb/kernel_modules/mandelbrot/mdb_kernel_common.h>


GLOBAL_VAR_INIT(const char*, name, "Mandelbrot generic kernel");
GLOBAL_VAR_INIT(const char*, ver_maj, "1");
GLOBAL_VAR_INIT(const char*, ver_min, "0");


int mdb_kernel_cpu_features(void)
{
        return 0;
}


/* Override the default compiler options for generating code
 * without any vector extension like mmx,sse,avx,fma, etc.
 * and force it to use x87 math coprocessor instead of the default sse on x86-64
 */
__attribute__((target("arch=x86-64,no-mmx,no-sse,no-sse2,no-sse3,no-ssse3,"
                      "no-sse4,no-avx,no-avx2,no-fma")))
void mdb_kernel_process_block(uint32_t x0, uint32_t x1,
                              uint32_t y0, uint32_t y1)
{
        const float scale = mdb.scale;
        const float shift_x = mdb.shift_x;
        const float shift_y = mdb.shift_y;
        static const float center = -0.5f;

        const float width_r = mdb.width_r;
        const float height_r = mdb.height_r;
        const float wxh = mdb.aspect_ratio;

        const uint32_t bailout = mdb.bailout;
        const float di = (float) 1 / bailout;
        uint32_t y, x;
        uint32_t i;
        float cy, cx, zx, zy, zx2, zy2, zxzy, mag2, norm_color;

        for (y = y0; y <= y1; ++y)
        {
                cy = (float) y * height_r;
                cy += center;
                cy *= scale;
                cy += shift_y;

                for (x = x0; x <= x1; ++x)
                {
                        cx = (float) x * width_r * wxh;
                        cx += center;
                        cx *= scale;
                        cx += shift_x;

                        zx = cx;
                        zy = cy;

                        for (i = 0; i < bailout; ++i)
                        {
                                zx2  = zx * zx;
                                zy2  = zy * zy;
                                zxzy = zx * zy;

                                zx = zx2 - zy2;
                                zx = zx  + cx;

                                zy = zxzy + zxzy;
                                zy = zy + cy;


                                zx2 = zx * zx;
                                zy2 = zy * zy;

                                mag2 = zx2 + zy2;

                                if(mag2 > 4.0f)
                                        break;
                        }

                        if (i == bailout)
                                i = 0;

                        norm_color = i * di;

                        surface_set_pixels(mdb.surf, x, y, 1, &norm_color);
                }
        }
}

