# MDB Fractal Generator

This project used to be just a CPU implementation of mandelbrot fractal.
Now it's a complete modular framework to render fractals and anything on CPU.

It supports the next features:
- Modular and configurable system.
- Convenient API for writing fractal generators ( Kernels ) as dynamically loadable modules.
- Multithreaded task scheduler that automatically splits and dispatches quants of kernel work across CPU and cores.
- Tools for benchmarking kernels performance.
- Real-time CPU rendering to screen using OpenGL to show
- Supports input events like keyboard and mouse in the kernels that allows creating controls in the render mode.
- Default render surface is 32-bit float texture allows creating HDR textures on the fly.
- Support direct render to HDR images (RGBE Radiance format)
- Includes example kernels out-of-box showing various technics like AVX2, FMA CPU Vector extensions, etc.
- Lean and fast code written in pure C.
- Using cmake as a build system.

Requirements:
- GCC and libc with C11 atomics support.
- CPU with AVX2 and FMA support to run specific kernels.
- GLWF and OpenGL >= 3.2 with GL_ARB_buffer_storage, GL_ARB_explicit_attrib_location, GL_ARB_explicit_uniform_location, GL_ARB_shading_language_420pack support.
- Only tested and targeted OS is Linux ( but may work on Windows with MinGW )

Build instructions:
- Clone or download sources to your local machine.
- Go to sources directory and create a build directory
- Select desired build options in mdb/config/config.cmake
- Go to build directory and invoke cmake -DCMAKE_BUILD_TYPE=Release ../
- invoke make
- if some modules failed to build check required compiler flags or disable it in the config.

Now you can run the application with some pre-existing kernel, type --help or run with the following options
./mdb -k mdb_avx2_fma --mode=render

