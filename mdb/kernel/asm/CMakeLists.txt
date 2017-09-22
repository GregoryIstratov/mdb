cmake_minimum_required(VERSION 3.8)
project(asm)

#set(CMAKE_CXX_STANDARD 11)

#set(CMAKE_C_FLAGS "-no-pie -march=native -static")
set(CMAKE_C_FLAGS "-std=gnu11 -D_GNU_SOURCE -no-pie -march=native -Wall -Wno-unused-function")

set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -O3 -flto -s")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g3")

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    message("Detected build type - ${CMAKE_BUILD_TYPE}")

    set(NASM_EXTRA_FLAGS "-gdwarf")
    set(KERNEL_LINK_EXTRA_FLAGS "")
elseif(${CMAKE_BUILD_TYPE} STREQUAL Release)
    message("Detected build type - ${CMAKE_BUILD_TYPE}")
    set(NASM_EXTRA_FLAGS "-Ox")
    set(KERNEL_LINK_EXTRA_FLAGS "-s")
endif ()

set(CMAKE_VERBOSE_MAKEFILE off)

#add_custom_target(asm_helpers ALL
#        COMMAND /usr/bin/gcc ${CMAKE_C_FLAGS} -o ${CMAKE_CURRENT_BINARY_DIR}/asm_helpers.o ${CMAKE_CURRENT_SOURCE_DIR}/asm_helpers.c
#        COMMENT "[${CMAKE_BUILD_TYPE}] Compiling asm_helpers.c to ${CMAKE_CURRENT_BINARY_DIR}/asm_helpers.o ..."
#        )

add_custom_target(compile_kernel ALL
        COMMAND /usr/bin/nasm ${NASM_EXTRA_FLAGS} -f elf64 -o ${CMAKE_CURRENT_SOURCE_DIR}/bin/kernel.o ${CMAKE_CURRENT_SOURCE_DIR}/kernel.asm
        COMMENT "[${CMAKE_BUILD_TYPE}] Compiling kernel.asm to ${CMAKE_CURRENT_SOURCE_DIR}/bin/kernel.o ..."
        VERBATIM
        )

#add_custom_target(build_kernel ALL
#        COMMAND /usr/bin/ld -static -lc -lgcc ${KERNEL_LINK_EXTRA_FLAGS} -o ${CMAKE_CURRENT_BINARY_DIR}/kernel ${CMAKE_CURRENT_BINARY_DIR}/kernel.o ${CMAKE_CURRENT_BINARY_DIR}/libasm_helpers.a
#        COMMENT "[${CMAKE_BUILD_TYPE}] Linking kernel.o to ${CMAKE_CURRENT_BINARY_DIR}/kernel ..."
#        VERBATIM
#        )

#include_directories(glad/include/)

#add_library(ogl_render STATIC ogl_render.c glad/src/glad.c)
#target_link_libraries(ogl_render dl glfw)

#add_library(asm_helpers STATIC asm_helpers.c)
#add_executable(kernel main.c ${CMAKE_CURRENT_SOURCE_DIR}/bin/kernel.o render_sched/rsched.c)
#target_link_libraries(kernel pthread ogl_render)

#add_dependencies(kernel compile_kernel)

#add_executable(hdconv hdrconv.c )



#add_library(mdbt_asm_kernel ${CMAKE_CURRENT_SOURCE_DIR}/bin/kernel.o)
#add_dependencies(mdbt_asm_kernel compile_kernel)