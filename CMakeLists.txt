cmake_minimum_required(VERSION 3.8)
project(mandelbrot)

set(CMAKE_VERBOSE_MAKEFILE off)

set(CMAKE_C_FLAGS "-std=gnu11 -D_GNU_SOURCE -no-pie -march=native -Wall -Wextra -Wno-unused-function")
set(CMAKE_C_FLAGS_DEBUG "-DDEBUG -ggdb3 -O0")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -fsanitize=undefined -fsanitize=leak")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -s -flto -ffast-math -funroll-loops -ftree-vectorize")

include_directories(mdb/render/glad/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    message("Detected build type - ${CMAKE_BUILD_TYPE}")

    set(NASM_EXTRA_FLAGS "-gdwarf")
    set(KERNEL_LINK_EXTRA_FLAGS "")
elseif(${CMAKE_BUILD_TYPE} STREQUAL Release)
    message("Detected build type - ${CMAKE_BUILD_TYPE}")
    set(NASM_EXTRA_FLAGS "-Ox")
    set(KERNEL_LINK_EXTRA_FLAGS "-s")
endif ()


add_custom_target(compile_kernel ALL
        COMMAND /usr/bin/nasm ${NASM_EXTRA_FLAGS} -f elf64
        -o ${CMAKE_CURRENT_SOURCE_DIR}/mdb/kernel/asm/bin/mdb_asm_kernel.o
        ${CMAKE_CURRENT_SOURCE_DIR}/mdb/kernel/asm/mdb_asm_kernel.asm
        COMMENT "[${CMAKE_BUILD_TYPE}] Compiling mdb_asm_kernel.asm to ${CMAKE_CURRENT_SOURCE_DIR}/bin/mdb_asm_kernel.o ..."
        VERBATIM
        )

set(SOURCE_FILES
        mdb/main.c
        mdb/kernel/asm/bin/mdb_asm_kernel.o
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
        mdb/kernel/bits/mdb_kernel_generic.c
        mdb/kernel/mdb_kernel_ext.c
        mdb/kernel/mdb_kernel_ext.h)

add_library(ogl_render SHARED mdb/render/glad/src/glad.c mdb/render/ogl_render.c)
target_link_libraries(ogl_render dl glfw)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders)

add_custom_command(
        TARGET ogl_render POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/mdb/render/shaders/*
        ${CMAKE_CURRENT_BINARY_DIR}/shaders/)

add_executable(mandelbrot ${SOURCE_FILES})
add_dependencies(mandelbrot compile_kernel)
target_link_libraries(mandelbrot pthread ogl_render)

