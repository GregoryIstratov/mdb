cmake_minimum_required(VERSION 3.8)
project(mandelbrot)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE  -no-pie -march=native -Wall -Wextra -Wno-unused-function")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -O0 -fno-inline")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto -ffast-math -ffinite-math-only -funroll-loops -ftree-vectorize")

set(CMAKE_C_FLAGS "-std=gnu11 -D_GNU_SOURCE -no-pie -march=native -Wall -Wextra -Wno-unused-function")
set(CMAKE_C_FLAGS_DEBUG "-DDEBUG -g3 -O0")
set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -O3 -ffast-math -ffinite-math-only -funroll-loops -ftree-vectorize")

include_directories(mdb/glad/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
#add_subdirectory(asm)

set(SOURCE_FILES
        mdb/main.c
        mdb/kernel/kernel_cpu.c
        mdb/kernel/kernel_cpu.h
        mdb/sched/rsched.c
        mdb/sched/rsched.h
        mdb/tools/args_parser.c
        mdb/tools/args_parser.h
        mdb/tools/utils.c
        mdb/tools/utils.h mdb/core/benchmark.c mdb/core/benchmark.h mdb/tools/atomic_x86.h mdb/tools/timer.h mdb/core/render.c mdb/core/render.h)

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