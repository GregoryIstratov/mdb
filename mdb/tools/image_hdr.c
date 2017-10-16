/* stb_image_write - v1.07 - public domain - http://nothings.org/stb/stb_image_write.h
   writes out PNG/BMP/TGA/JPEG/HDR images to C stdio - Sean Barrett 2010-2015
                                     no warranty implied; use at your own risk

   Before #including,

       #define STB_IMAGE_WRITE_IMPLEMENTATION

   in the file that you want to have the implementation.

   Will probably not work correctly with strict-aliasing optimizations.

ABOUT:

   This header file is a library for writing images to C stdio. It could be
   adapted to write to memory or a general streaming interface; let me know.

   The PNG output is not optimal; it is 20-50% larger than the file
   written by a decent optimizing implementation. This library is designed
   for source code compactness and simplicity, not optimal image file size
   or run-time performance.

BUILDING:

   You can #define STBIW_ASSERT(x) before the #include to avoid using assert.h.
   You can #define STBIW_MALLOC(), STBIW_REALLOC(), and STBIW_FREE() to replace
   malloc,realloc,free.
   You can define STBIW_MEMMOVE() to replace memmove()

USAGE:

   There are four functions, one for each image file format:

     int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
     int stbi_write_bmp(char const *filename, int w, int h, int comp, const void *data);
     int stbi_write_tga(char const *filename, int w, int h, int comp, const void *data);
     int stbi_write_hdr(char const *filename, int w, int h, int comp, const float *data);
     int stbi_write_jpg(char const *filename, int w, int h, int comp, const float *data);

   There are also four equivalent functions that use an arbitrary write function. You are
   expected to open/close your file-equivalent before and after calling these:

     int stbi_write_png_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data, int stride_in_bytes);
     int stbi_write_bmp_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data);
     int stbi_write_tga_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data);
     int stbi_write_hdr_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const float *data);
     int stbi_write_jpg_to_func(stbi_write_func *func, void *context, int x, int y, int comp, const void *data, int quality);

   where the callback is:
      void stbi_write_func(void *context, void *data, int size);

   You can define STBI_WRITE_NO_STDIO to disable the file variant of these
   functions, so the library will not use stdio.h at all. However, this will
   also disable HDR writing, because it requires stdio for formatted output.

   Each function returns 0 on failure and non-0 on success.

   The functions create an image file defined by the parameters. The image
   is a rectangle of pixels stored from left-to-right, top-to-bottom.
   Each pixel contains 'comp' channels of data stored interleaved with 8-bits
   per channel, in the following order: 1=Y, 2=YA, 3=RGB, 4=RGBA. (Y is
   monochrome color.) The rectangle is 'w' pixels wide and 'h' pixels tall.
   The *data pointer points to the first byte of the top-left-most pixel.
   For PNG, "stride_in_bytes" is the distance in bytes from the first byte of
   a row of pixels to the first byte of the next row of pixels.

   PNG creates output files with the same number of components as the input.
   The BMP format expands Y to RGB in the file format and does not
   output alpha.

   PNG supports writing rectangles of data even when the bytes storing rows of
   data are not consecutive in memory (e.g. sub-rectangles of a larger image),
   by supplying the stride between the beginning of adjacent rows. The other
   formats do not. (Thus you cannot write a native-format BMP through the BMP
   writer, both because it is in BGR order and because it may have padding
   at the end of the line.)

   HDR expects linear float data. Since the format is always 32-bit rgb(e)
   data, alpha (if provided) is discarded, and for monochrome data it is
   replicated across all three channels.

   TGA supports RLE or non-RLE compressed data. To use non-RLE-compressed
   data, set the global variable 'stbi_write_tga_with_rle' to 0.

   JPEG does ignore alpha channels in input data; quality is between 1 and 100.
   Higher quality looks better but results in a bigger image.
   JPEG baseline (no JPEG progressive).

CREDITS:

   PNG/BMP/TGA
      Sean Barrett
   HDR
      Baldur Karlsson
   TGA monochrome:
      Jean-Sebastien Guay
   misc enhancements:
      Tim Kelsey
   TGA RLE
      Alan Hickman
   initial file IO callback implementation
      Emmanuel Julien
   JPEG
      Jon Olick (original jo_jpeg.cpp code)
      Daniel Gibson
   bugfixes:
      github:Chribba
      Guillaume Chereau
      github:jry2
      github:romigrou
      Sergio Gonzalez
      Jonas Karlsson
      Filip Wasil
      Thatcher Ulrich
      github:poppolopoppo
      Patrick Boettcher

LICENSE

  See end of file for license information.

*/

#ifndef INCLUDE_STB_IMAGE_WRITE_H
#define INCLUDE_STB_IMAGE_WRITE_H

#ifdef __cplusplus
extern "C" {
#endif

#define STB_IMAGE_WRITE_STATIC

#define STBIWDEF static


#ifndef STBI_WRITE_NO_STDIO

STBIWDEF int
stbi_write_hdr(char const* filename, int w, int h, int comp, const float* data);

#endif

typedef void stbi_write_func(void* context, void* data, int size);

STBIWDEF int
stbi_write_hdr_to_func(stbi_write_func* func, void* context, int w, int h,
                       int comp, const float* data);

#ifdef __cplusplus
}
#endif

#endif


#ifdef _WIN32
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif
#endif

#ifndef STBI_WRITE_NO_STDIO

#include <stdio.h>

#endif

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if defined(STBIW_MALLOC) && defined(STBIW_FREE) && (defined(STBIW_REALLOC) || defined(STBIW_REALLOC_SIZED))

#elif !defined(STBIW_MALLOC) && !defined(STBIW_FREE) && !defined(STBIW_REALLOC) && !defined(STBIW_REALLOC_SIZED)

#else
#error "Must define all or none of STBIW_MALLOC, STBIW_FREE, and STBIW_REALLOC (or STBIW_REALLOC_SIZED)."
#endif

#ifndef STBIW_MALLOC
#define STBIW_MALLOC(sz)        malloc(sz)
#define STBIW_REALLOC(p, newsz)  realloc(p,newsz)
#define STBIW_FREE(p)           free(p)
#endif

#ifndef STBIW_REALLOC_SIZED
#define STBIW_REALLOC_SIZED(p, oldsz, newsz) STBIW_REALLOC(p,newsz)
#endif


#ifndef STBIW_MEMMOVE
#define STBIW_MEMMOVE(a, b, sz) memmove(a,b,sz)
#endif


#ifndef STBIW_ASSERT

#include <assert.h>

#define STBIW_ASSERT(x) assert(x)
#endif

#define STBIW_UCHAR(x) (unsigned char) ((x) & 0xff)

typedef struct
{
        stbi_write_func* func;
        void* context;
} stbi__write_context;

// initialize a callback-based context
static void
stbi__start_write_callbacks(stbi__write_context* s, stbi_write_func* c,
                            void* context)
{
        s->func = c;
        s->context = context;
}

#ifndef STBI_WRITE_NO_STDIO

static void stbi__stdio_write(void* context, void* data, int size)
{
        fwrite(data, 1, size, (FILE*) context);
}

static int stbi__start_write_file(stbi__write_context* s, const char* filename)
{
        FILE* f = fopen(filename, "wb");
        stbi__start_write_callbacks(s, stbi__stdio_write, (void*) f);
        return f != NULL;
}

static void stbi__end_write_file(stbi__write_context* s)
{
        fclose((FILE*) s->context);
}

#endif // !STBI_WRITE_NO_STDIO

typedef unsigned int stbiw_uint32;
typedef int stb_image_write_test[sizeof(stbiw_uint32) == 4 ? 1 : -1];

static void stbiw__writefv(stbi__write_context* s, const char* fmt, va_list v)
{
        while(*fmt)
        {
                switch(*fmt++)
                {
                        case ' ':
                                break;
                        case '1':
                        {
                                unsigned char x = STBIW_UCHAR(va_arg(v,
                                                                      int));
                                s->func(s->context, &x, 1);
                                break;
                        }
                        case '2':
                        {
                                int x = va_arg(v, int);
                                unsigned char b[2];
                                b[0] = STBIW_UCHAR(x);
                                b[1] = STBIW_UCHAR(x >> 8);
                                s->func(s->context, b, 2);
                                break;
                        }
                        case '4':
                        {
                                stbiw_uint32 x = va_arg(v, int);
                                unsigned char b[4];
                                b[0] = STBIW_UCHAR(x);
                                b[1] = STBIW_UCHAR(x >> 8);
                                b[2] = STBIW_UCHAR(x >> 16);
                                b[3] = STBIW_UCHAR(x >> 24);
                                s->func(s->context, b, 4);
                                break;
                        }
                        default:
                                STBIW_ASSERT(0);
                                return;
                }
        }
}

static void stbiw__writef(stbi__write_context* s, const char* fmt, ...)
{
        va_list v;
        va_start(v, fmt);
        stbiw__writefv(s, fmt, v);
        va_end(v);
}

static void stbiw__putc(stbi__write_context* s, unsigned char c)
{
        s->func(s->context, &c, 1);
}

static void
stbiw__write3(stbi__write_context* s, unsigned char a, unsigned char b,
              unsigned char c)
{
        unsigned char arr[3];
        arr[0] = a, arr[1] = b, arr[2] = c;
        s->func(s->context, arr, 3);
}

static void
stbiw__write_pixel(stbi__write_context* s, int rgb_dir, int comp,
                   int write_alpha, int expand_mono, unsigned char* d)
{
        unsigned char bg[3] = {255, 0, 255}, px[3];
        int k;

        if(write_alpha < 0)
                s->func(s->context, &d[comp - 1], 1);

        switch(comp)
        {
                case 2: // 2 pixels = mono + alpha, alpha is written separately, so same as 1-channel case
                case 1:
                        if(expand_mono)
                                stbiw__write3(s, d[0], d[0],
                                              d[0]); // monochrome bmp
                        else
                                s->func(s->context, d, 1);  // monochrome TGA
                        break;
                case 4:
                        if(!write_alpha)
                        {
                                // composite against pink background
                                for(k = 0; k < 3; ++k)
                                        px[k] = bg[k] +
                                                ((d[k] - bg[k]) * d[3]) / 255;
                                stbiw__write3(s, px[1 - rgb_dir], px[1],
                                              px[1 + rgb_dir]);
                                break;
                        }
                        /* FALLTHROUGH */
                case 3:
                        stbiw__write3(s, d[1 - rgb_dir], d[1], d[1 + rgb_dir]);
                        break;
        }
        if(write_alpha > 0)
                s->func(s->context, &d[comp - 1], 1);
}

static void
stbiw__write_pixels(stbi__write_context* s, int rgb_dir, int vdir, int x, int y,
                    int comp, void* data, int write_alpha,
                    int scanline_pad, int expand_mono)
{
        stbiw_uint32 zero = 0;
        int i, j, j_end;

        if(y <= 0)
                return;

        if(vdir < 0)
                j_end = -1, j = y - 1;
        else
                j_end = y, j = 0;

        for(; j != j_end; j += vdir)
        {
                for(i = 0; i < x; ++i)
                {
                        unsigned char* d =
                                (unsigned char*) data + (j * x + i) * comp;
                        stbiw__write_pixel(s, rgb_dir, comp, write_alpha,
                                           expand_mono, d);
                }
                s->func(s->context, &zero, scanline_pad);
        }
}

static int
stbiw__outfile(stbi__write_context* s, int rgb_dir, int vdir, int x, int y,
               int comp, int expand_mono, void* data,
               int alpha, int pad, const char* fmt, ...)
{
        if(y < 0 || x < 0)
        {
                return 0;
        }
        else
        {
                va_list v;
                va_start(v, fmt);
                stbiw__writefv(s, fmt, v);
                va_end(v);
                stbiw__write_pixels(s, rgb_dir, vdir, x, y, comp, data, alpha,
                                    pad, expand_mono);
                return 1;
        }
}


// *************************************************************************************************
// Radiance RGBE HDR writer
// by Baldur Karlsson

#define stbiw__max(a, b)  ((a) > (b) ? (a) : (b))

void stbiw__linear_to_rgbe(unsigned char* rgbe, float* linear)
{
        int exponent;
        float maxcomp = stbiw__max(linear[0], stbiw__max(linear[1], linear[2]));

        if(maxcomp < 1e-32f)
        {
                rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
        }
        else
        {
                float normalize =
                        (float) frexp(maxcomp, &exponent) * 256.0f / maxcomp;

                rgbe[0] = (unsigned char) (linear[0] * normalize);
                rgbe[1] = (unsigned char) (linear[1] * normalize);
                rgbe[2] = (unsigned char) (linear[2] * normalize);
                rgbe[3] = (unsigned char) (exponent + 128);
        }
}

void stbiw__write_run_data(stbi__write_context* s, int length,
                           unsigned char databyte)
{
        unsigned char lengthbyte = STBIW_UCHAR(length + 128);
        STBIW_ASSERT(length + 128 <= 255);
        s->func(s->context, &lengthbyte, 1);
        s->func(s->context, &databyte, 1);
}

void
stbiw__write_dump_data(stbi__write_context* s, int length, unsigned char* data)
{
        unsigned char lengthbyte = STBIW_UCHAR(length);
        STBIW_ASSERT(length <=
                     128); // inconsistent with spec but consistent with official code
        s->func(s->context, &lengthbyte, 1);
        s->func(s->context, data, length);
}

void stbiw__write_hdr_scanline(stbi__write_context* s, int width, int ncomp,
                               unsigned char* scratch, float* scanline)
{
        unsigned char scanlineheader[4] = {2, 2, 0, 0};
        unsigned char rgbe[4];
        float linear[3];
        int x;

        scanlineheader[2] = (width & 0xff00) >> 8;
        scanlineheader[3] = (width & 0x00ff);

        /* skip RLE for images too small or large */
        if(width < 8 || width >= 32768)
        {
                for(x = 0; x < width; x++)
                {
                        switch(ncomp)
                        {
                                case 4: /* fallthrough */
                                case 3:
                                        linear[2] = scanline[x * ncomp + 2];
                                        linear[1] = scanline[x * ncomp + 1];
                                        linear[0] = scanline[x * ncomp + 0];
                                        break;
                                default:
                                        linear[0] = linear[1] = linear[2] = scanline[
                                                x * ncomp + 0];
                                        break;
                        }
                        stbiw__linear_to_rgbe(rgbe, linear);
                        s->func(s->context, rgbe, 4);
                }
        }
        else
        {
                int c, r;
                /* encode into scratch buffer */
                for(x = 0; x < width; x++)
                {
                        switch(ncomp)
                        {
                                case 4: /* fallthrough */
                                case 3:
                                        linear[2] = scanline[x * ncomp + 2];
                                        linear[1] = scanline[x * ncomp + 1];
                                        linear[0] = scanline[x * ncomp + 0];
                                        break;
                                default:
                                        linear[0] = linear[1] = linear[2] = scanline[
                                                x * ncomp + 0];
                                        break;
                        }
                        stbiw__linear_to_rgbe(rgbe, linear);
                        scratch[x + width * 0] = rgbe[0];
                        scratch[x + width * 1] = rgbe[1];
                        scratch[x + width * 2] = rgbe[2];
                        scratch[x + width * 3] = rgbe[3];
                }

                s->func(s->context, scanlineheader, 4);

                /* RLE each component separately */
                for(c = 0; c < 4; c++)
                {
                        unsigned char* comp = &scratch[width * c];

                        x = 0;
                        while(x < width)
                        {
                                // find first run
                                r = x;
                                while(r + 2 < width)
                                {
                                        if(comp[r] == comp[r + 1] &&
                                           comp[r] == comp[r + 2])
                                                break;
                                        ++r;
                                }
                                if(r + 2 >= width)
                                        r = width;
                                // dump up to first run
                                while(x < r)
                                {
                                        int len = r - x;
                                        if(len > 128) len = 128;
                                        stbiw__write_dump_data(s, len,
                                                               &comp[x]);
                                        x += len;
                                }
                                // if there's a run, output it
                                if(r + 2 < width)
                                { // same test as what we break out of in search loop, so only true if we break'd
                                        // find next byte after run
                                        while(r < width && comp[r] == comp[x])
                                                ++r;
                                        // output run up to r
                                        while(x < r)
                                        {
                                                int len = r - x;
                                                if(len > 127) len = 127;
                                                stbiw__write_run_data(s, len,
                                                                      comp[x]);
                                                x += len;
                                        }
                                }
                        }
                }
        }
}

static int
stbi_write_hdr_core(stbi__write_context* s, int x, int y, int comp, float* data)
{
        if(y <= 0 || x <= 0 || data == NULL)
                return 0;
        else
        {
                // Each component is stored separately. Allocate scratch space for full output scanline.
                unsigned char* scratch = (unsigned char*) STBIW_MALLOC(x * 4);
                int i, len;
                char buffer[128];
                char header[] = "#?RADIANCE\n# Written by stb_image_write.h\nFORMAT=32-bit_rle_rgbe\n";
                s->func(s->context, header, sizeof(header) - 1);

                len = sprintf(buffer,
                              "EXPOSURE=          1.0000000000000\n\n-Y %d +X %d\n",
                              y, x);
                s->func(s->context, buffer, len);

                for(i = 0; i < y; i++)
                        stbiw__write_hdr_scanline(s, x, comp, scratch,
                                                  data + comp * i * x);
                STBIW_FREE(scratch);
                return 1;
        }
}

STBIWDEF int
stbi_write_hdr_to_func(stbi_write_func* func, void* context, int x, int y,
                       int comp, const float* data)
{
        stbi__write_context s;
        stbi__start_write_callbacks(&s, func, context);
        return stbi_write_hdr_core(&s, x, y, comp, (float*) data);
}

STBIWDEF int
stbi_write_hdr(char const* filename, int x, int y, int comp, const float* data)
{
        stbi__write_context s;
        if(stbi__start_write_file(&s, filename))
        {
                int r = stbi_write_hdr_core(&s, x, y, comp, (float*) data);
                stbi__end_write_file(&s);
                return r;
        }
        else
                return 0;
}


#include "image_hdr.h"


int image_hdr_save_r32(const char* filename, uint32_t width, uint32_t height,
                       float* f32data)
{
        return !stbi_write_hdr(filename, width, height, 1, f32data);
}
