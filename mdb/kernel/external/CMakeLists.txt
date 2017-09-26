cmake_minimum_required(VERSION 3.8)
project(kernel_dynamic C)

set(CMAKE_VERBOSE_MAKEFILE off)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})


message("Current src dir ${CMAKE_CURRENT_SOURCE_DIR}")
message("Current bin dir ${CMAKE_CURRENT_BINARY_DIR}")
message("Top bin dir ${CMAKE_BINARY_DIR}")

file(GLOB files "kernels/*.c")
foreach(file ${files})
    get_filename_component(kname ${file} NAME_WE)
    message("Found external kernel [${kname}]")

    add_library(${kname} MODULE ${file})
    set_target_properties(${kname} PROPERTIES PREFIX "")
endforeach()