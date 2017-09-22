#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <cmath>
#include <sstream>

#define __CL_ENABLE_EXCEPTIONS

#include <CL/cl.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

#include <immintrin.h>
#include <iomanip>
#include <atomic>

#include <sys/time.h>
#include <sys/resource.h>

#include <time.h>
#include <stdint.h>

#include <sched.h>

#include <log.hpp>
#include <atomic.hpp>

#include <complex>
#include <thread>

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>

#include <csignal>
#include <functional>

#include <asm/kernel.h>
#include <unordered_map>
#include <sched/rsched.h>
#include <cstdint>


static constexpr uint64_t KiB = 1024;
static constexpr uint64_t MiB = 1024 * KiB;
static constexpr uint64_t GiB = 1024 * MiB;

class bytes_hr
{
public:
    explicit bytes_hr(uint64_t _size)
            : size(_size)
    {}

    uint64_t size;
};


inline std::ostream& operator<<(std::ostream& os, bytes_hr bytes)
{
    auto flags = os.flags();

    if (bytes.size < KiB)
    {
        os << bytes.size << " Bytes";
    }
    else if (bytes.size < MiB)
    {
        os << std::fixed << std::setprecision(3) << ((double) bytes.size / KiB) << " KiB";
    }
    else if (bytes.size < GiB)
    {
        os << std::fixed << std::setprecision(3) << ((double) bytes.size / MiB) << " MiB";
    }
    else
    {
        os << std::fixed << std::setprecision(6) << ((double) bytes.size / GiB) << " GiB";
    }

    os.flags(flags);

    return os;
}

__always_inline const char* bool_enabled(bool b)
{
    return b ? "Enabled" : "Disabled";
}

class perf_timer
{
public:
    using int_type = uint64_t;

    explicit perf_timer(int clock_type = CLOCK_MONOTONIC)
            : clock_type_(clock_type)
    {

    }

    void start() noexcept
    {
        if (clock_gettime(clock_type_, &start_))
        {
            LOG_ERROR << "clock_gettime failed to take the time and switched to fallback";
        }
    }

    void stop() noexcept
    {
        if (clock_gettime(clock_type_, &end_))
        {
            LOG_ERROR << "clock_gettime failed to take the time and switched to fallback";
        }
    }

    int_type nanoseconds() const noexcept
    {
        auto total_start = get_total_ns(&start_);
        auto total_end = get_total_ns(&end_);

        auto total = total_end - total_start;

        return total;
    }

    int_type milliseconds() const noexcept
    {
        auto total_start = get_total_ms(&start_);
        auto total_end = get_total_ms(&end_);

        auto total = total_end - total_start;

        return total;
    }

    double seconds() const noexcept
    {
        auto total_start = get_total_sec(&start_);
        auto total_end = get_total_sec(&end_);

        auto total = total_end - total_start;

        return total;
    }

    static double ns_to_ms(int_type ns)
    {
        return (double) ns / NS_IN_MS;
    }

    static double ns_to_sec(int_type ns)
    {
        return (double) ns / NS_IN_SEC;
    }


private:
    static constexpr int_type NS_IN_SEC = 1000000000;
    static constexpr int_type MCS_IN_SEC = 1000000;
    static constexpr int_type MS_IN_SEC = 1000;
    static constexpr int_type NS_IN_MS = 1000000;

    inline void timespec_diff(const timespec* ts0, const timespec* ts1, timespec* ts_res) const noexcept
    {
        ts_res->tv_sec = ts1->tv_sec - ts0->tv_sec;
        ts_res->tv_nsec = ts1->tv_nsec - ts0->tv_nsec;
    }


    inline int_type get_total_ns(const timespec* ts) const noexcept
    {
        if (ts->tv_sec == 0)
            return (int_type) ts->tv_nsec;

        int_type total_ns = (int_type) ts->tv_sec * NS_IN_SEC;
        total_ns += ts->tv_nsec;

        return total_ns;
    }

    inline int_type get_total_ms(const timespec* ts) const noexcept
    {
        if (ts->tv_sec == 0)
            return (int_type) ts->tv_sec / NS_IN_MS;

        int_type total_ms = (int_type) ts->tv_sec * MS_IN_SEC;
        total_ms += (int_type) ts->tv_nsec / NS_IN_MS;

        return total_ms;
    }

    inline double get_total_sec(const timespec* ts) const noexcept
    {
        if (ts->tv_sec == 0)
            return (double) ts->tv_nsec / NS_IN_SEC;

        double total_sec = (double) ts->tv_nsec / NS_IN_SEC;
        total_sec += (double) ts->tv_sec;

        return total_sec;
    }

private:
    int clock_type_;
    timespec start_, end_;
};

enum class cl_use_device
{
    cpu,
    gpu
};

static void mandelbrot_opencl(int WIDTH, int HEIGHT, int bailout, cl_use_device use_device, unsigned char* data)
{
    try
    {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);

        std::vector<cl::Device> platformDevices;
        if (use_device == cl_use_device::gpu)
            platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &platformDevices);
        else
            platforms[1].getDevices(CL_DEVICE_TYPE_CPU, &platformDevices);

        cl::Context context(platformDevices);
        auto contextDevices = context.getInfo<CL_CONTEXT_DEVICES>();
        for (const auto& dev : contextDevices)
        {
            LOG_INFO << "Using " << dev.getInfo<CL_DEVICE_NAME>();;
        }


        std::string opts;
        opts += "-cl-fast-relaxed-math";
        opts += " -DBAILOUT=" + std::to_string(bailout);
        opts += " -DHEIGHT=" + std::to_string(HEIGHT);
        opts += " -DWIDTH=" + std::to_string(WIDTH);

        std::ifstream programFile;
        programFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        programFile.open("mandelbrot2.cl");


        std::string programString(std::istreambuf_iterator<char>(programFile), (std::istreambuf_iterator<char>()));

        cl::Program::Sources source(1, std::make_pair(programString.c_str(), programString.length() + 1));
        cl::Program program(context, source);
        try
        {
            program.build(contextDevices, opts.c_str());
        }
        catch (cl::Error& e)
        {
            // FIXME may not be the device that failed
            LOG_INFO << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(contextDevices[0]);
        }

        LOG_INFO << program.getBuildInfo<CL_PROGRAM_BUILD_OPTIONS>(contextDevices[0]);
        LOG_INFO << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(contextDevices[0]);

        cl::Kernel mandelbrot(program, "mandelbrot");

        // command queues
        std::vector<cl::CommandQueue> queues;
        for (const auto& device : contextDevices)
        {
            cl::CommandQueue queue(context, device, CL_QUEUE_PROFILING_ENABLE);
            queues.push_back(queue);
        }

        auto start = std::chrono::high_resolution_clock::now();

        // partition the "y" dimension
        int i = 0;
        size_t workItemsPerQueue =
                (size_t) HEIGHT / queues.size(); // FIXME requires work size to be evenly divisible by number of queues
        std::vector<cl::Buffer> outputs;

        for (auto& queue : queues)
        {
            cl::NDRange offset(0, 0); //i*workItemsPerQueue);
            cl::NDRange global_size(WIDTH, workItemsPerQueue);

            // storage for results per device
            cl_int err = CL_SUCCESS;
            cl::Buffer output(context, CL_MEM_WRITE_ONLY, (size_t) WIDTH * workItemsPerQueue, nullptr, &err);
            mandelbrot.setArg(0, output);
            mandelbrot.setArg(1, (int) (i * workItemsPerQueue));
            outputs.push_back(output);

            queue.enqueueNDRangeKernel(mandelbrot, offset, global_size);
            queue.enqueueBarrierWithWaitList();

            LOG_INFO << "enqueued range " << i * workItemsPerQueue << " of length " << workItemsPerQueue;

            i++;
        }

        // read results
        std::vector<cl::Event> readWaitList;

        i = 0;
        for (auto& queue : queues)
        {
            size_t offset = i * WIDTH * workItemsPerQueue;
            cl::Event readDoneEvent;
            queue.enqueueReadBuffer(outputs[i], CL_FALSE, 0, WIDTH * workItemsPerQueue, &(data[offset]), nullptr,
                                    &readDoneEvent);

            // NOTE: can't push onto vector until the event is valid, since it will be copied
            readWaitList.push_back(readDoneEvent);

            i++;
        }

        cl::Event::waitForEvents(readWaitList);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;

        LOG_INFO << "computation took " << elapsed_seconds.count() << "s";

        //stbi_write_png("mandelbrot_cl.png", WIDTH, HEIGHT, 1, results, WIDTH);


    }
    catch (const cl::Error& e)
    {
        LOG_ERROR << e.what() << ": Error code " << e.err();
        exit(EXIT_FAILURE);
    }
    catch (const std::system_error& e)
    {
        LOG_ERROR << "Error:" << strerror(errno);
        exit(EXIT_FAILURE);
    }
}

struct PX_RGB32F {};
struct PX_R32F {};
struct PX_R8 {};


template<typename T>
struct pixel;

template<>
struct pixel<PX_R8>
{
    union
    {
        uint8_t data[1];
        uint8_t r;
    };

    inline void broadcast(float norm_color)
    {
        r = (uint8_t)(norm_color * 255);
    }

    static constexpr const char* name()
    {
        return "PX_R8";
    }
};


template<>
struct pixel<PX_RGB32F>
{
    union
    {
        uint8_t data[sizeof(float) * 4];
        struct
        {
            float r;
            float g;
            float b;
            float a;
        };

    };

    __always_inline void broadcast(float norm_color)
    {
        r = norm_color;
        g = norm_color;
        b = norm_color;
    }

    static constexpr const char* name()
    {
        return "PX_RGB32F";
    }
};

template<>
struct pixel<PX_R32F>
{
    union
    {
        uint8_t data[sizeof(float)];
        struct
        {
            float r;
        };

    };

    inline void broadcast(float norm_color)
    {
        r = norm_color;
    }

    static constexpr const char* name()
    {
        return "PX_R32F";
    }
};

using pixel_rgb32f = pixel<PX_RGB32F>;
using pixel_r8     = pixel<PX_R8>;
using pixel_r32f   = pixel<PX_R32F>;

struct no_copyable
{
    no_copyable(const no_copyable&) = delete;

    no_copyable& operator=(const no_copyable&) = delete;
};

struct no_moveable
{
    no_moveable(no_moveable&&) = delete;

    no_moveable& operator=(no_moveable&&) = delete;
};


inline void log_print_separator_line()
{
    LOG_INFO << "------------------------------------------";
}


template<typename PixelFormat>
class RenderSurface
{
public:
    using pixel_t = pixel<PixelFormat>;

    RenderSurface(int width, int height)
    {
        width_  = width;
        height_ = height;

        size_t size = (size_t) width * height;
        size_t mem_size = size * sizeof(pixel_t);

        const char* pixel_format_name = pixel_t::name();

        log_print_separator_line();
        LOG_INFO << "ImgBuffer size      : " << width << "x" << height;
        LOG_INFO << "ImgBuffer px format : " << pixel_format_name << " @ " << (sizeof(pixel_t) * 8) << " BPP";
        LOG_INFO << "ImgBuffer mem size  : " << (mem_size) << " Bytes / " << (mem_size / 1024) << " KiB / "
                 << (mem_size / 1024 / 1024) << " MiB";

        //size_t align = 2 * 1024 * 1024; // 2MB for allocating in Transparent Huge Pages
        //size_t align = 64; //Align for fitting in to the beginning of the cache line
        size_t align = 32; //Required to avx aligned memory operations

        int res = posix_memalign((void**) &data_, align, mem_size);
        if (res)
        {
            if (ENOMEM == res)
                LOG_FATAL << "There was insufficient memory available to satisfy the request.";
            if (EINVAL == res)
                LOG_FATAL << "alignment is not a power of two multiple of sizeof (void *).";

            throw std::bad_alloc{};
        }

        if((size_t)data_ % align)
        {
            LOG_WARNING << "Allocated data is not aligned";
        }


        // iterate over all elements to warm up pages for avoiding minor page faults
        static tbb::affinity_partitioner ap;
        tbb::parallel_for(tbb::blocked_range<size_t>(0, size - 1),
                          [this](const tbb::blocked_range<size_t>& block)
                          {
                              // iterate over all elements to warm up pages
                              auto data = data_;
                              for (size_t i = block.begin(); i != block.end(); ++i)
                              {
                                  data[i].broadcast(0);
                              }

                          }, ap);
    }

    void release()
    {
        if (data_)
        {
            free(data_);
            data_ = nullptr;
        }
    }


    __always_inline int width() const
    {
        return width_;
    }

    __always_inline int height() const
    {
        return height_;
    }


    __always_inline const pixel_t* data() const
    {
        return data_;
    }

    __always_inline pixel_t* data()
    {
        return data_;
    }


    inline pixel_t& pixel_at(int x, int y)
    {
        return data_[y*height_+x];
    }


    __always_inline void set_pixel_avx_v(int x, int y, __m256 v_i_norm);
    

private:
    struct alignas(64)
    {
        union
        {
            uint8_t cache_line[64];
            struct
            {
                pixel_t* data_;
                int width_;
                int height_;
            };
        };
    };
};

template<>
void RenderSurface<PX_R8>::set_pixel_avx_v(int x, int y, __m256 v_i_norm)
{
    v_i_norm = _mm256_mul_ps(v_i_norm, _mm256_set1_ps(0xFF));

    __m256i v_col = _mm256_cvtps_epi32(v_i_norm);

    alignas(32) uint32_t colors[8];
    _mm256_store_si256((__m256i*) colors, v_col);

    pixel_at(x + 0, y).r = (uint8_t) colors[0];
    pixel_at(x + 1, y).r = (uint8_t) colors[1];
    pixel_at(x + 2, y).r = (uint8_t) colors[2];
    pixel_at(x + 3, y).r = (uint8_t) colors[3];
    pixel_at(x + 4, y).r = (uint8_t) colors[4];
    pixel_at(x + 5, y).r = (uint8_t) colors[5];
    pixel_at(x + 6, y).r = (uint8_t) colors[6];
    pixel_at(x + 7, y).r = (uint8_t) colors[7];
}

template<>
void RenderSurface<PX_RGB32F>::set_pixel_avx_v(int x, int y, __m256 v_i_norm)
{
    alignas(32) float colors[8];
    _mm256_store_ps(colors, v_i_norm);

    pixel_at(x + 0, y).broadcast(colors[0]);
    pixel_at(x + 1, y).broadcast(colors[1]);
    pixel_at(x + 2, y).broadcast(colors[2]);
    pixel_at(x + 3, y).broadcast(colors[3]);
    pixel_at(x + 4, y).broadcast(colors[4]);
    pixel_at(x + 5, y).broadcast(colors[5]);
    pixel_at(x + 6, y).broadcast(colors[6]);
    pixel_at(x + 7, y).broadcast(colors[7]);
}

template<>
void RenderSurface<PX_R32F>::set_pixel_avx_v(int x, int y, __m256 v_i_norm)
{
    float* p = (float*)&(data_[y*height_+x]);
//    if((size_t)p % 32)
//    {
//        LOG_WARNING << "Pixel at "<<x<<"x"<<y<<" is not 32 byte aligned : "<<(void*)p;
//    }
    _mm256_storeu_ps(p, v_i_norm);
}

namespace iset_t
{

struct avx {};
struct avx_fma {};
struct generic {};
struct native  {};
struct asm_avx_fma {};

}

enum class iset
{
    avx,
    avx_fma,
    asm_avx_afm,
    native,
    generic
};

inline const char* iset_to_string(iset i)
{
    switch(i)
    {
        case iset::avx: return "AVX";
        case iset::avx_fma: return "AVX FMA3";
        case iset::asm_avx_afm: return "ASM AVX FMA3";
        case iset::native: return "NATIVE";
        case iset::generic: return "GENERIC";
    }

    return "";
}

inline iset string_to_iset(const std::string& s)
{
    static const std::unordered_map<std::string, iset> ops = {
            {"avx", iset::avx },
            {"avx_fma", iset::avx_fma},
            {"asm_avx_fma", iset::asm_avx_afm},
            {"native", iset::native},
            {"generic", iset::generic}
    };

    auto found = ops.find(s);

    if(found != ops.end())
    {
        return found->second;
    }
    else
    {
        throw std::runtime_error("[string_to_iset] Can't parse input: " + s);
    }

}

template<typename FloatT, typename InstructionSet, typename PixelFormat>
class mandelbrot_cpu_kernel
{
public:

    template<typename U = InstructionSet, typename std::enable_if<!std::is_same<U, iset_t::asm_avx_fma>::value>::type* = nullptr>
    mandelbrot_cpu_kernel(RenderSurface<PixelFormat>* surface,
                          FloatT shift_x, FloatT shift_y, FloatT scale, int bailout)
            : surface_(surface), shift_x_(shift_x), shift_y_(shift_y), scale_(scale), bailout_(bailout)
    {
        width_r_  = (FloatT)1 / surface_->width();
        height_r_ = (FloatT)1 / surface_->height();
        wxh_      = (FloatT)surface_->width() / surface_->height();
    }

    template<typename U = InstructionSet, typename std::enable_if<std::is_same<U, iset_t::asm_avx_fma>::value>::type* = nullptr>
    mandelbrot_cpu_kernel(RenderSurface<PixelFormat>* surface,
                          FloatT shift_x, FloatT shift_y, FloatT scale, int bailout)
            : surface_(surface), shift_x_(shift_x), shift_y_(shift_y), scale_(scale), bailout_(bailout)
    {
        mdbt_set_size((uint64_t)surface->width(), (uint64_t)surface->height());
        mdbt_set_surface((float*)surface->data());
        mdbt_set_bailout((uint32_t)bailout);
        mdbt_set_shift(shift_x, shift_y);
        mdbt_set_scale(scale);
        mdbt_compute_transpose_offset();
    }

    template<typename U = InstructionSet>
    __always_inline
    typename std::enable_if_t<std::is_same<U, iset_t::asm_avx_fma>::value>
    process_block(int x0, int x1, int y0, int y1)
    {
        mdbt_kernel(x0, x1, y0, y1);
    }

    template<typename U = InstructionSet>
    inline __attribute__((target("tune=generic")))
    typename std::enable_if_t<std::is_same<U, iset_t::generic>::value>
    process_block(int x0, int x1, int y0, int y1)
    {
        process_block_generic(x0, x1, y0, y1);
    };


    template<typename U = InstructionSet>
    __always_inline
    typename std::enable_if_t<std::is_same<U, iset_t::native>::value>
    process_block(int x0, int x1, int y0, int y1)
    {
        process_block_generic(x0, x1, y0, y1);
    };

    template<typename U = InstructionSet>
    //inline __attribute__((target("tune=generic")))
    inline
    typename std::enable_if_t<std::is_same<U, iset_t::generic>::value || std::is_same<U, iset_t::native>::value>
    process_block_generic(int x0, int x1, int y0, int y1)
    {
        const FloatT scale = scale_;
        const FloatT shift_x = shift_x_;
        const FloatT shift_y = shift_y_;
        static const FloatT center = -0.5;

        const FloatT width_r = width_r_;
        const FloatT height_r = height_r_;
        const FloatT wxh = wxh_;

        const auto bailout = bailout_;
        const FloatT di = (FloatT) 1 / bailout;

        for (int y = y0; y <= y1; ++y)
        {
            FloatT cy = (FloatT) y * height_r;
            cy += center;
            cy *= scale;
            cy += shift_y;

            for (int x = x0; x <= x1; ++x)
            {
                FloatT cx = (FloatT) x * width_r * wxh;
                cx += center;
                cx *= scale;
                cx += shift_x;

                const std::complex<FloatT> C(cx, cy);

                std::complex<FloatT> z(C);
                int i;
                for (i = 0; i < bailout; ++i)
                {
                    z = z * z + C;
                    if (std::norm(z) > 4)
                        break;
                }

                if (i == bailout)
                    i = 0;

                float norm_col = (float)(i * di);

                surface_->pixel_at(x, y).broadcast(norm_col);
            }
        }
    }


    template<typename U = InstructionSet>
    __always_inline
    typename std::enable_if_t<std::is_same<U, iset_t::avx>::value>
    process_block(int x0, int x1, int y0, int y1)
    {
        __m256 v_scale   = _mm256_set1_ps(scale_);
        __m256 v_shift_x = _mm256_set1_ps(shift_x_);
        __m256 v_shift_y = _mm256_set1_ps(shift_y_);
        __m256 v_center = _mm256_set1_ps(-0.5f);

        const auto bailout = bailout_;

        __m256 v_width_r = _mm256_set1_ps(width_r_);
        __m256 v_height_r = _mm256_set1_ps(height_r_);
        __m256 v_wxh = _mm256_set1_ps(wxh_);

        __m256 v_bound2 = _mm256_set1_ps(4);
        __m256 v_one = _mm256_set1_ps(1);

        for (int y = y0; y <= y1; ++y)
        {
            __m256 v_cy = _mm256_set1_ps(y);
            v_cy = _mm256_mul_ps(v_cy, v_height_r);
            v_cy = _mm256_add_ps(v_cy, v_center);
            v_cy = _mm256_mul_ps(v_cy, v_scale);
            v_cy = _mm256_add_ps(v_cy, v_shift_y);

            for (int x = x0; x < x1; x += 8)
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

                surface_->set_pixel_avx_v(x, y, v_i);

            }
        }
    };


    template<typename U = InstructionSet>
    __always_inline
    typename std::enable_if_t<std::is_same<U, iset_t::avx_fma>::value>
    process_block(int x0, int x1, int y0, int y1)
    {
        __m256 v_scale   = _mm256_set1_ps(scale_);
        __m256 v_shift_x = _mm256_set1_ps(shift_x_);
        __m256 v_shift_y = _mm256_set1_ps(shift_y_);
        __m256 v_center  = _mm256_set1_ps(-0.5f);

        const auto bailout = bailout_;

        __m256 v_width_r = _mm256_set1_ps(width_r_);
        __m256 v_height_r = _mm256_set1_ps(height_r_);
        __m256 v_wxh = _mm256_set1_ps(wxh_);

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

                surface_->set_pixel_avx_v(x, y, v_i);

            }
        }
    }



private:
    RenderSurface<PixelFormat>* surface_;
    FloatT shift_x_;
    FloatT shift_y_;
    FloatT scale_;
    FloatT width_r_;
    FloatT height_r_;
    FloatT wxh_;
    int    bailout_;
};

struct BlockSize
{
    int x;
    int y;

    BlockSize(int _x, int _y) : x(_x), y(_y) {}

    BlockSize(const std::string& s)
    {
        int res = sscanf(s.c_str(), "%dx%d", &x, &y);
        if(res == 1)
        {
            y = x;
        }
        else if(res == 0)
        {
            throw std::runtime_error{"[BlockSize] Can't parse the input '"+s+"'"};
        }

    }
};


struct mandelbrot_cpu_run_opts
{
    bool parallel;
    bool benchmark;
    int  benchmark_iterations;
    iset is;

};


template<typename FloatT, typename PixelFormat = PX_R8>
class mandelbrot_cpu
{
public:
    using pixel_t = pixel<PixelFormat>;

    template<typename InstructionSet>
    using kernel_t = mandelbrot_cpu_kernel<FloatT, InstructionSet, PixelFormat>;

    mandelbrot_cpu(int width, int height, int bailout, BlockSize block_size, int block_log)
            : width_(width), height_(height), bailout_(bailout), block_size_(block_size), block_log_(block_log),
              img_buffer(width_, height_),
              total_block_time_(0), min_block_time_(UINT64_MAX), max_block_time_(0), blocks_count_(0),
              sched_(tbb::task_scheduler_init::automatic),
              kernel_native_(&img_buffer, shift_x_, shift_y_, scale_, bailout_),
              kernel_generic_(&img_buffer, shift_x_, shift_y_, scale_, bailout_),
              kernel_avx_(&img_buffer, shift_x_, shift_y_, scale_, bailout_),
              kernel_avx_fma_(&img_buffer, shift_x_, shift_y_, scale_, bailout_),
              kernel_asm_avx_fma_(&img_buffer, shift_x_, shift_y_, scale_, bailout_)
    {
    }


    ~mandelbrot_cpu()
    {
        img_buffer.release();
    }

    void run(const mandelbrot_cpu_run_opts& opts)
    {

        bool benchmark = opts.benchmark;
        bool parallel  = opts.parallel;
        iset is        = opts.is;

        const int cpu_cores = parallel ? std::thread::hardware_concurrency() : 1;
        const int texture_size = (int) lround(sqrtf((float) (width_ * height_)));


        const int block_size_x = block_size_.x;
        const int block_size_y = block_size_.y;
        //const size_t data_size = width_ * height_ * sizeof(pixel_t);


        LOG_INFO << "Run mode            : " << (benchmark? "Benchmark" : "Normal");

        if(benchmark)
        LOG_INFO << "Iterations          : " << opts.benchmark_iterations;

        LOG_INFO << "Threading mode      : " << (parallel ? "Parallel" : "Single");
        LOG_INFO << "Threads             : " << cpu_cores;
        LOG_INFO << "Instruction set     : " << iset_to_string(is);
        LOG_INFO << "Texture size^2      : " << texture_size;
        LOG_INFO << "Block size          : " << block_size_x << "x" << block_size_y;
        LOG_INFO << "Data per block      : " << bytes_hr(block_size_x * block_size_y * sizeof(pixel_t));

        rsched_create(&sched_, width_ * height_ / block_size_x, cpu_cores,  )
        tbb::blocked_range2d<int> r2d(0, width_ - 1, (const size_t) block_size_x, 0, height_ - 1,
                                      (const size_t) block_size_y);


        kernel_run_result result;



        if(is == iset::avx_fma)
            run_kernel(kernel_avx_fma_, r2d, opts, result);
        else if(is == iset::asm_avx_afm)
            run_kernel(kernel_asm_avx_fma_, r2d, opts, result);
        else if(is == iset::avx)
            run_kernel(kernel_avx_, r2d, opts, result);
        else if(is == iset::generic)
            run_kernel(kernel_generic_, r2d, opts, result);
        else if(is == iset::native)
            run_kernel(kernel_native_, r2d, opts, result);


        int frames = result.frames;
        perf_timer tm = result.tm;

        auto block_ms = perf_timer::ns_to_ms(stde::atomic::load(total_block_time_) / cpu_cores);
        auto min_block_ms = perf_timer::ns_to_ms(stde::atomic::load(min_block_time_));
        auto max_block_ms = perf_timer::ns_to_ms(stde::atomic::load(max_block_time_));

        LOG_INFO << "Total blocks        : " << blocks_count_;
        LOG_INFO << "Avg block time      : " << std::fixed << std::setprecision(4) << (block_ms / blocks_count_)
                 << " ms";
        LOG_INFO << "Min block time      : " << std::fixed << std::setprecision(4) << min_block_ms << " ms";
        LOG_INFO << "Max block time      : " << std::fixed << std::setprecision(4) << max_block_ms << " ms";
        LOG_INFO << "Total block time    : " << std::fixed << std::setprecision(4) << (block_ms / 1000.0) << " sec";
        LOG_INFO << "Total execution time: " << std::fixed << std::setprecision(4) << tm.seconds() << " sec";
        LOG_INFO << "Total frames        : " << frames;
        LOG_INFO << "Avg FPS             : " << ((double) frames / tm.seconds());
    }


    void stop_benchmark()
    {
        stde::atomic::store(benchmark_flag_, false);
    }

    int width() const
    { return width_; }

    int height() const
    { return height_; }

    auto data() const
    {
        return img_buffer.data();
    }

private:
    struct kernel_run_result
    {
        perf_timer tm;
        int frames;

    };

    template<typename Kernel>
    void run_kernel(Kernel& kernel, tbb::blocked_range2d<int>& block, const mandelbrot_cpu_run_opts& opts,
                    kernel_run_result& result)
    {
        perf_timer tm{};
        int frames = 0;

        if (opts.benchmark)
        {
            //stde::atomic::store(benchmark_flag_, true);
            int benchmark_iterations = opts.benchmark_iterations;

            tm.start();

            if (opts.parallel)
            {
                //while (stde::atomic::load(benchmark_flag_) && frames < benchmark_iterations)
                while (frames < benchmark_iterations)
                {
                    process_blocked_parallel(block, kernel);
                    ++frames;
                }
            }
            else
            {
                //while (stde::atomic::load(benchmark_flag_) && frames < benchmark_iterations)
                while (frames < benchmark_iterations)
                {
                    process_blocked(block, kernel);
                    ++frames;
                }
            }

            tm.stop();
        }
        else
        {
            tm.start();

            if (opts.parallel)
                process_blocked_parallel(block, kernel);
            else
                process_blocked(block, kernel);

            tm.stop();

            ++frames;
        }

        result.frames = frames;
        result.tm     = tm;
    }


    template<typename Kernel>
    void process_kernel_block(int x0, int x1, int y0, int y1, Kernel kernel)
    {
        perf_timer tm{};
        tm.start();

        kernel.process_block(x0, x1, y0, y1);

        tm.stop();

        uint64_t elapsed_time = tm.nanoseconds();

        auto min_time = stde::atomic::load(min_block_time_);
        while (min_time > elapsed_time)
        {
            if (min_block_time_.compare_exchange_weak(min_time, elapsed_time, std::memory_order_release,
                                                      std::memory_order_acquire))
                break;
        }

        auto max_time = stde::atomic::load(max_block_time_);
        while (max_time < elapsed_time)
        {
            if (max_block_time_.compare_exchange_weak(max_time, elapsed_time, std::memory_order_release,
                                                      std::memory_order_acquire))
                break;
        }

        total_block_time_.fetch_add(elapsed_time, std::memory_order_relaxed);
        blocks_count_.fetch_add(1, std::memory_order_relaxed);

        if (block_log_)
            print_thread_statistic(x1 - x0, y1 - y0, x0, y0, elapsed_time);

    }

    template<typename Kernel>
    void process_blocked(tbb::blocked_range2d<int>& block, Kernel& kernel)
    {
        if (block.is_divisible())
        {
            tbb::blocked_range2d<int> block2(block, tbb::split());

            process_blocked(block, kernel);
            process_blocked(block2, kernel);
        }
        else
        {
            process_kernel_block<typename std::add_lvalue_reference<Kernel>::type>
                    (block.rows().begin(), block.rows().end(),
                    block.cols().begin(), block.cols().end(), kernel);
        }
    }


    template<typename Kernel>
    void process_blocked_parallel(tbb::blocked_range2d<int>& block, Kernel& kernel)
    {
        static tbb::simple_partitioner sp;

        tbb::parallel_for(block, [&](const tbb::blocked_range2d<int>& r)
        {
            process_kernel_block<Kernel>(r.rows().begin(), r.rows().end(),
                                 r.cols().begin(), r.cols().end(), kernel);
        }, sp);

    }

    inline void print_thread_statistic(int x_len, int y_len, int x0, int y0, uint64_t elapsed_time)
    {
        static std::mutex mtx;
        mtx.lock();
        LOG_INFO
                << std::setw(4)
                << std::right
                << x_len
                << "x"
                << std::setw(4)
                << std::left
                << y_len
                << " "
                << std::left << std::setw(8) << (x_len * y_len)
                << " "
                << std::setw(4) << std::right
                << x0
                << " "
                << std::setw(4) << std::left
                << y0
                << " "
                << std::setw(4) << std::right << perf_timer::ns_to_ms(elapsed_time) << " ms";
        mtx.unlock();
    }

private:
    const int width_;
    const int height_;
    const int bailout_;
    const BlockSize block_size_;
    const int block_log_;
    RenderSurface<PixelFormat> img_buffer;
    std::atomic<uint64_t> total_block_time_;
    std::atomic<uint64_t> min_block_time_;
    std::atomic<uint64_t> max_block_time_;
    std::atomic<uint64_t> blocks_count_;
    std::atomic_bool benchmark_flag_;
    rsched sched_;

    kernel_t<iset_t::native> kernel_native_;
    kernel_t<iset_t::generic> kernel_generic_;
    kernel_t<iset_t::avx> kernel_avx_;
    kernel_t<iset_t::avx_fma> kernel_avx_fma_;
    kernel_t<iset_t::asm_avx_fma> kernel_asm_avx_fma_;

    static constexpr const FloatT scale_ = 0.00188964;
    static constexpr const FloatT shift_x_ = -1.347385054652062;
    static constexpr const FloatT shift_y_ = 0.063483549665202;

};


void test_perf_timer()
{
    perf_timer tm{};
    timespec dur, rem;
    dur.tv_sec = 0;
    dur.tv_nsec = 1000000;

    tm.start();
    nanosleep(&dur, &rem);
    tm.stop();

    if (rem.tv_sec || rem.tv_nsec)
    {
        LOG_ERROR << "Failed to sleep on the specified time " << rem.tv_sec << " sec " << rem.tv_nsec << " ns remained";
    }

    LOG_INFO << "sleep takes " << tm.seconds() << " sec " << tm.milliseconds() << " ms " << tm.nanoseconds() << " ns";

}


std::function<void()> stop_benchmark_fn;

void signal_handler(int)
{
    stop_benchmark_fn();
}

template<typename T>
inline void save_image(const mandelbrot_cpu<T, PX_RGB32F>& mb, const std::string& filename)
{
    auto data = mb.data();

    std::string filename_ext = filename + ".hdr";

    LOG_INFO << "Writing data to ''" << filename_ext << "'...";
    perf_timer tm;
    tm.start();

    stbi_write_hdr(filename_ext.c_str(), mb.width(), mb.height(), 4, (const float*)data);

    tm.stop();
    LOG_INFO << "done for " << tm.seconds() << " sec";
}

template<typename T>
inline void save_image(const mandelbrot_cpu<T, PX_R32F>& mb, const std::string& filename)
{
    auto data = mb.data();

    std::string filename_ext = filename + ".hdr";

    log_print_separator_line();

    LOG_INFO << "Output type         : Radiance RGBE HDR File";
    LOG_INFO << "Filename            : " << filename_ext;
    perf_timer tm;
    tm.start();

    stbi_write_hdr(filename_ext.c_str(), mb.width(), mb.height(), 1, (const float*)data);

    tm.stop();
    LOG_INFO << "Encoding time       : " << tm.seconds() << " sec";
}

template<typename T>
inline void save_image(const mandelbrot_cpu<T, PX_R8>& mb, const std::string& filename)
{
    auto data = mb.data();
    std::string filename_ext = filename + ".png";

    LOG_INFO << "Writing data to ''" << filename_ext << "'...";

    perf_timer tm;
    tm.start();
    //stbi_write_bmp("mandelbrot.bmp", WIDTH, HEIGHT, 1, data);
    stbi_write_png(filename_ext.c_str(), mb.width(), mb.height(), 1, data, mb.width());
    tm.stop();
    LOG_INFO << "done for " << tm.seconds() << " sec";
}


#define TRACE_CLOCK_RESOLUTION(clock_id) \
{\
    timespec __clock_resolution;\
    if(clock_getres(clock_id, &__clock_resolution))\
    {\
        LOG_WARNING << "Can't get a resolution of the clock: "<<strerror(errno);\
    }\
    else\
    {\
        LOG_TRACE << "Resolution of "<<#clock_id<<": "<<__clock_resolution.tv_nsec<<" ns";\
    }\
}

void adjust_process()
{
    sched_param param;
    param.sched_priority = 0;
    sched_setscheduler(0, SCHED_BATCH, &param);

    setpriority(PRIO_PROCESS, 0, -20);
}

int main(int argc, const char** argv)
{
    adjust_process();

    int width;
    int height;
    int quad;
    int bailout;
    std::string block_size_s;
    int block_log;
    bool benchmark = false;
    int benchmark_iterations = 1;
    bool parallel = true;
    bool hdr = false;
    bool fp_double = false;
    std::string iset_opt;
    std::string output;

    namespace po = boost::program_options;

    try
    {
        stde::logging::init();
        stde::logging::add_stdout_sink();


        po::options_description desc;
        desc.add_options()
                ("help", "Help message")
                ("width,w", po::value<int>(&width)->default_value(1024), "Image width")
                ("height,h", po::value<int>(&height)->default_value(1024), "Image height")
                ("quad,x", po::value<int>(&quad)->default_value(1024), "Image width x height")
                ("bailout,i", po::value<int>(&bailout)->default_value(256), "Bailout / Max iteration depth")
                ("block-size,b", po::value<std::string>(&block_size_s)->default_value("16x512"), "Block size NxM per thread")
                ("block-log,l", po::value<int>(&block_log)->default_value(0), "Show block execution statistics")
                ("benchmark",po::value<int>(), "Benchmark mode")
                ("serial", "Non parallel mode, use only singe core")
                ("iset", po::value<std::string>(&iset_opt)->default_value("generic"), "Instruction set")
                ("hdr",
                 "Use RGB 32 bit fp unit per pixel, if output is specified image saved in *.hrd format preserving high definition range of color (default: 8 bit per pixel grayscale)")
                ("fp-double", "Set using 64 bit floating point unit (default: float 32 bit)")
                ("output,o", po::value<std::string>(&output)->default_value("mandelbrot"), "Output file");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help"))
        {
            std::cout << desc << std::endl;
            return 0;
        }

        if (vm.count("benchmark"))
        {
            benchmark = true;
            auto found = vm.find("benchmark");
            const auto& value = found->second;
            if(value.empty())
            {
                benchmark_iterations = 100;
            }
            else
            {
                benchmark_iterations = value.as<int>();
            }

        }
        if (vm.count("serial")) parallel = false;
        if (vm.count("hdr")) hdr = true;
        if (vm.count("fp-double")) fp_double = true;
        if (vm.count("quad")) { width = quad; height = quad; }



        if (sysconf(_SC_MONOTONIC_CLOCK) <= 0)
        {
            throw std::runtime_error{"Monotonic clock is not supported on this system"};
        }


        //TRACE_CLOCK_RESOLUTION(CLOCK_MONOTONIC)

        iset is = string_to_iset(iset_opt);

        log_print_separator_line();

        BlockSize block_size(block_size_s);

        LOG_INFO << "Resolution          : " << width << "x" << height;

        if(is != iset::generic)
        {
            if(fp_double && width % 4)
            {
                throw std::runtime_error{"If AVX enabled width must be product of 4 on fp-double mode"};
            }
            if(width % 8)
            {
                throw std::runtime_error{"If AVX enabled width must be product of 8"};
            }
        }

        LOG_INFO << "HDR Mode            : " << bool_enabled(hdr);
        LOG_INFO << "FP-Double           : " << bool_enabled(fp_double);
        LOG_INFO << "Bailout             : " << bailout;

        mandelbrot_cpu_run_opts opts;
        opts.parallel  = parallel;
        opts.benchmark = benchmark;
        opts.is        = is;
        opts.benchmark_iterations = benchmark_iterations;

        std::signal(SIGINT, signal_handler);

        if (hdr && fp_double)
        {
            mandelbrot_cpu<double, PX_R32F> mb_cpu(width, height, bailout, block_size, block_log);

            stop_benchmark_fn = [&]()
            {
                mb_cpu.stop_benchmark();
            };

            mb_cpu.run(opts);

            if (!benchmark)
                save_image(mb_cpu, "mandelbrot");
        }

        if (!hdr && fp_double)
        {
            mandelbrot_cpu<double, PX_R8> mb_cpu(width, height, bailout, block_size, block_log);

            stop_benchmark_fn = [&]()
            {
                mb_cpu.stop_benchmark();
            };

            mb_cpu.run(opts);

            if (!benchmark)
                save_image(mb_cpu, "mandelbrot");
        }

        if (hdr && !fp_double)
        {
            mandelbrot_cpu<float, PX_R32F> mb_cpu(width, height, bailout, block_size, block_log);

            stop_benchmark_fn = [&]()
            {
                mb_cpu.stop_benchmark();
            };

            mb_cpu.run(opts);

            if (!benchmark)
                save_image(mb_cpu, "mandelbrot");
        }

        if (!hdr && !fp_double)
        {
            mandelbrot_cpu<float, PX_R8> mb_cpu(width, height, bailout, block_size, block_log);

            stop_benchmark_fn = [&]()
            {
                mb_cpu.stop_benchmark();
            };

            mb_cpu.run(opts);

            if (!benchmark)
                save_image(mb_cpu, "mandelbrot");

        }

    }
    catch (const std::exception& e)
    {
        LOG_FATAL << "Error: "<<e.what();
        stde::logging::shutdown();
        return -1;
    }

    stde::logging::shutdown();

    return 0;
}
