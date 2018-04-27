# MDB CPU-Render Framework

###MDB is a framework for making computation on CPU.

MDB framework was designed and developed to give developers an easy way for making very fast computations on CPU.

All you need is to write your own program so-called "module" or "kernel" (implying computing module) through the given API in C ( but it's still possible to pick another language )

MDB framework has many features and settings for tuning, debugging, profiling and benchmarking your module.

MDB Framework was written in plain C so it's very fast, lean and clean, it primarily uses OpenGL for rendering backend, but also can be extended with Direct3D or other backends.

There are a few built-in modules written with various techniques like using AVX instructions and regular versions for performance comparing, which can be also used as examples for writing your own modules.  
  
The framework is mainly designed for creating and showing 2D surfaces in real-time on screen or making snapshots of them, but can be also used for your purposes. 

####Supporting features:
- Modular and configurable system.
- Convenient API for writing computing kernels as dynamically loadable modules.
- Multi-threaded task scheduler that automatically splits and dispatches quants (small pieces) of kernel work across CPU and cores.
- Tools for benchmarking kernel performance.
- Real-time CPU rendering to screen using OpenGL.
- GLSL shaders for further image processing.
- Keyboard and Mouse input events in the render mode.
- Default render surface is a 32-bit float texture, it allows creating HDR textures on the fly.
- Direct rendering to HDR images (RGBE Radiance format)
- Example kernels out-of-box with various techniques like AVX2, FMA CPU Vector extensions, etc.
- Lean and fast code written in plain C.
- Cmake as a build system.

####Requirements:
- GCC >=~ 4.8
- CPU with AVX2 and FMA support to run specific kernels.
- GLWF3 and OpenGL >= 3.2 with GL_ARB_buffer_storage if you want to run it in the render mode.
- Targeted mainly to Linux (Windows can run it as well but with some restrictions, MacOS hasn't tested yet.) 

####Build instructions:
- Clone or download sources to your local machine.
- Go to the sources directory and create a build directory
- (Optional) Select desired build options in mdb/config/config.cmake
- Go to the build directory and type: _cmake -DCMAKE_BUILD_TYPE=Release ../_
- type: _make_

Now application can be run with some pre-existing kernel, type: _--help_ or run it with the following options for example:\
_./mdb -k mdb_avx2_fma --mode=render_\
or\
_./mdb -k mdb_avx2_fma --mode=benchmark --benchmark-runs=1000_

#####Development platform:
- **OS**: Arch Linux, Kernel 4.12.13 Ck-Patch, GCC 7.2, Glibc 2.26 
- **CPU**: Intel(R) Core(TM) i5-4670K
- **GPU**: Nvidia GTX 580, Proprietary driver / Integrated Intel CPU graphics.

#####Tested platforms:
- Arch Linux Kernel 4.13 GCC 7.2 on Intel Celeron B820 1.7 GHz, Integrated graphics.
- Ubuntu 14.04 LTS, GCC 4.8.4 on QEMU-KVM on dev platform
- Ubuntu 16.04 LTS, GCC 5.4.0 on QEMU-KVM on dev platform
