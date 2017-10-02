cmake_minimum_required(VERSION 3.8)
project(modules C)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

message("Modules root dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message("Modules binary dir: ${CMAKE_CURRENT_BINARY_DIR}")

if(MDB_ENABLE_AVX_FMA_ASM_KERNEL)
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        message("Detected build type - ${CMAKE_BUILD_TYPE}")

        set(NASM_EXTRA_FLAGS "-gdwarf")
        set(KERNEL_LINK_EXTRA_FLAGS "")
        set(NASM_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/kernels/mandelbrot/asm/bin/debug")
    elseif(${CMAKE_BUILD_TYPE} STREQUAL Release)
        message("Detected build type - ${CMAKE_BUILD_TYPE}")
        set(NASM_EXTRA_FLAGS "-Ox")
        set(KERNEL_LINK_EXTRA_FLAGS "-s")
        set(NASM_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/kernels/mandelbrot/asm/bin/release")
    endif ()


    add_custom_target(compile_asm_kernel ALL
            COMMAND /usr/bin/nasm ${NASM_EXTRA_FLAGS} -f elf64
            -o ${NASM_BUILD_DIR}/mdb_asm_kernel.o
            ${CMAKE_CURRENT_SOURCE_DIR}/kernels/mandelbrot/asm/mdb_asm_kernel.asm
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

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/kernels/mandelbrot/*.c")
foreach(file ${files})

    get_filename_component(kname ${file} NAME_WE)

    if((${kname} STREQUAL mdb_avx2_fma) AND NOT ${MDB_ENABLE_AVX2_FMA_KERNEL})
        message("Found kernel [${kname}] status DISABLED")
        continue()
    endif()

    if((${kname} STREQUAL mdb_avx2) AND NOT ${MDB_ENABLE_AVX2_KERNEL})
        message("Found kernel [${kname}] status DISABLED")
        continue()
    endif()

    if((${kname} STREQUAL mdb_native) AND NOT ${MDB_ENABLE_NATIVE_KERNEL})
        message("Found kernel [${kname}] status DISABLED")
        continue()
    endif()


    message("Found kernel [${kname}] status ENABLED")

    add_library(${kname} MODULE
            ${file}
            kernel_module.c
            kernels/mandelbrot/bits/mdb_kernel_common.c
            kernels/mandelbrot/bits/mdb_kernel_common.h
            )
    #target_link_libraries(${kname} kernel_module)
    set_target_properties(${kname} PROPERTIES PREFIX "")
    set_target_properties(${kname}
            PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY
            ${CMAKE_CURRENT_BINARY_DIR}/kernels)
endforeach()