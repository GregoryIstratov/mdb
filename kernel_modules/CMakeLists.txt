cmake_minimum_required(VERSION 2.8)
project(modules C)

include_directories(.)

message("Modules root dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message("Modules binary dir: ${CMAKE_CURRENT_BINARY_DIR}")

if(CONFIG_MDB_AVX_FMA_ASM_KERNEL)
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        message("Detected build type - ${CMAKE_BUILD_TYPE}")

        set(NASM_EXTRA_FLAGS "-gdwarf")
        set(KERNEL_LINK_EXTRA_FLAGS "")
        set(NASM_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles")
    elseif(${CMAKE_BUILD_TYPE} STREQUAL Release)
        message("Detected build type - ${CMAKE_BUILD_TYPE}")
        set(NASM_EXTRA_FLAGS "-Ox")
        set(KERNEL_LINK_EXTRA_FLAGS "-s")
        set(NASM_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles")
    endif ()


    add_custom_target(compile_asm_kernel ALL
            COMMAND /usr/bin/nasm ${NASM_EXTRA_FLAGS} -f elf64
            -o ${NASM_BUILD_DIR}/mdb_asm_kernel.o
            mandelbrot/mdb_asm_kernel.asm
            COMMENT "[${CMAKE_BUILD_TYPE}] Compiling mdb_asm_kernel.asm to ${NASM_BUILD_DIR}/mdb_asm_kernel.o ..."
            VERBATIM
            )

    add_library(mdb_avx2_fma_asm MODULE ${NASM_BUILD_DIR}/mdb_asm_kernel.o)
    set_target_properties(mdb_avx2_fma_asm PROPERTIES LINKER_LANGUAGE C)
    set_target_properties(mdb_avx2_fma_asm PROPERTIES PREFIX "")
    set_target_properties(mdb_avx2_fma_asm
            PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY
            ${CMAKE_CURRENT_BINARY_DIR}/kernels)
    add_dependencies(mdb_avx2_fma_asm compile_asm_kernel)
endif()


function(add_kernel kname kfiles)

    message("Kernel [${kname}] will be built")

    set(ksources ${ARGV})
    list(REMOVE_ITEM ksources ${kname})

    #message("Kernel [${kname}] sources: ${ksources}")

    add_library(${kname} MODULE ${ksources} kernel_module.c klog.c ../tools/log.c)

    set_target_properties(${kname} PROPERTIES PREFIX "")
    set_target_properties(${kname} PROPERTIES SUFFIX ".kern")

    set_target_properties(${kname}
            PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY
            ${CMAKE_CURRENT_BINARY_DIR}/kernels)

endfunction()


if(${CONFIG_MDB_AVX2_FMA_KERNEL})

    add_kernel(mdb_avx2_fma
            mandelbrot/mdb_avx2_fma.c
            mandelbrot/mdb_kernel_common.c
            mandelbrot/mdb_kernel_common.h
            )
    target_compile_options(mdb_avx2_fma PRIVATE -mavx2 -mfma)

endif()

if(${CONFIG_MDB_AVX2_KERNEL})

    add_kernel(mdb_avx2
            mandelbrot/mdb_avx2.c
            mandelbrot/mdb_kernel_common.c
            mandelbrot/mdb_kernel_common.h
            )
    target_compile_options(mdb_avx2 PRIVATE -mavx2)
endif()

if(${CONFIG_MDB_NATIVE_KERNEL})

    add_kernel(mdb_native
            mandelbrot/mdb_native.c
            mandelbrot/mdb_kernel_common.c
            mandelbrot/mdb_kernel_common.h
            )
    target_compile_options(mdb_native PRIVATE -march=native)

    add_kernel(mdb_native_vec
            mandelbrot/mdb_native_vec.c
            mandelbrot/mdb_kernel_common.c
            mandelbrot/mdb_kernel_common.h
            )
    target_compile_options(mdb_native_vec PRIVATE -m64 -mtune=generic)
endif()

add_kernel(mdb_generic
        mandelbrot/mdb_generic.c
        mandelbrot/mdb_kernel_common.c
        mandelbrot/mdb_kernel_common.h
        )

