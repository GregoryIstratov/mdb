cmake_minimum_required(VERSION 3.8)
project(kernel_dynamic C)

set(CMAKE_VERBOSE_MAKEFILE off)

#set(CMAKE_C_FLAGS "-std=gnu11 -D_GNU_SOURCE -fPIC -march=native -Wall -Wextra -Wno-unused-function")
#set(CMAKE_C_FLAGS_DEBUG "-DDEBUG -ggdb3 -O0")
#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=leak")
#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=undefined")
#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address")
#set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -s -flto -ffast-math -funroll-loops -ftree-vectorize")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})


message("Current src dir ${CMAKE_CURRENT_SOURCE_DIR}")
message("Current bin dir ${CMAKE_CURRENT_BINARY_DIR}")
message("Top bin dir ${CMAKE_BINARY_DIR}")

file(GLOB files "kernels/*.c")
foreach(file ${files})
    get_filename_component(kname ${file} NAME_WE)
    message("Found external kernel [${kname}]")

    add_library(${kname} SHARED ${file})
    set_target_properties(${kname} PROPERTIES PREFIX "")
endforeach()