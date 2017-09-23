cmake_minimum_required(VERSION 3.8)
project(mandelbrot)

set(CMAKE_C_FLAGS "-std=gnu11 -D_GNU_SOURCE -no-pie -march=native -Wall -Wextra -Wno-unused-function")
set(CMAKE_C_FLAGS_DEBUG "-DDEBUG -ggdb3 -O0")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -fsanitize=undefined -fsanitize=leak")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -s -flto -ffast-math -funroll-loops -ftree-vectorize")

include_directories(mdb/glad/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
#add_subdirectory(asm)

set(SOURCE_FILES
        mdb/main.c
        mdb/kernel/mdb_kernel.c
        mdb/kernel/mdb_kernel.h
        mdb/sched/rsched.c
        mdb/sched/rsched.h
        mdb/tools/args_parser.c
        mdb/tools/args_parser.h
        mdb/tools/utils.c
        mdb/tools/utils.h
        mdb/core/benchmark.c
        mdb/core/benchmark.h
        mdb/tools/atomic_x86.h
        mdb/tools/timer.h
        mdb/core/render.c
        mdb/core/render.h
        mdb/config/config.h
        mdb/kernel/bits/mdb_kernel.h
        mdb/kernel/bits/mdb_kernel_avx2_fma.c
        mdb/kernel/bits/surface.h
        mdb/kernel/bits/mdb_kernel_avx2.c
        mdb/kernel/bits/mdb_kernel_common.h
        mdb/kernel/bits/mdb_kernel_native.c
        mdb/kernel/bits/mdb_kernel_generic.c)

add_library(ogl_render SHARED mdb/glad/src/glad.c mdb/render/ogl_render.c)
target_link_libraries(ogl_render dl glfw)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders)

add_custom_command(
        TARGET ogl_render POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/mdb/shaders/*
        ${CMAKE_CURRENT_BINARY_DIR}/shaders/)

add_executable(mandelbrot ${SOURCE_FILES})
target_link_libraries(mandelbrot pthread ogl_render)

#add_dependencies(kernel compile_kernel)