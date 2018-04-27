# =====================================================
# This file contains compile time options             #
# Make sure your compiler and hardware support        #
# features that you want to enable like AVX and FMA   #
#                                                     #
# To enable option just set it ON to disable OFF      #
# =====================================================

#======================================================
# Arg parser parameters                               #
#======================================================

# Enable debug information in arg parser
# Note that arg parse doesn't use default logging system
# It uses only stdout and strerr for output
#
set(CONFIG_ARG_PARSER_DEBUG Off)

#======================================================
# Logging parameters                                  #
#======================================================

# Enable colored messages by adding special codes
# works only in supported terminals
#
set(CONFIG_LOG_COLOR Off)

# Show time of a log record
set(CONFIG_LOG_TIME On)

# Show date of a log record
set(CONFIG_LOG_DATE On)

# Show thread ID where logging record is coming from"
set(CONFIG_LOG_THREAD On)

# Show a function where is message from
set(CONFIG_LOG_FUNC On)

# Show a full source file path and a line number
# where is a log record coming from
# Useful when debugging
#
set(CONFIG_LOG_PATH On)

# If enabled protect logging functions with a mutex
set(CONFIG_LOG_MULTITHREADING On)

#======================================================
# Scheduler parameters                                #
#======================================================

# Enable scheduler debug output
# This may lead to a huge performance impact and a massive verbose output.
# Don't enable it unless you know what you're doing.
# This option has an effect only in a debug build
#
set(CONFIG_RSCHED_DEBUG Off)

# Enable scheduler profiling
# This records various performance timers and shows statistics on exit
#
set(CONFIG_RSCHED_PROFILE Off)

#======================================================
# Kernel parameters                                   #
#======================================================

# Turn on debugging in the kernel.
# This may lead to a huge performance impact and a massive verbose output.
# Don't enable it unless you know what you're doing.
# This option has an effect only in a debug build
#
set(CONFIG_MDB_KERNEL_DEBUG Off)

# Enable building a kernel that using avx2 and fma instruction sets.
# Written in intrinsics to maximize performance gain of vectorisation CPU extension.
# Your CPU and compiler must support AVX2 and FMA features.
#
set(CONFIG_MDB_AVX2_FMA_KERNEL On)

# Enable building a kernel that using avx2 instruction set ( without using FMA )
# thus this may be slightly slower than that one above which using FMA.
# This kernel is also written in intrinsics to maximize performance gain of vectorisation CPU extension.
# Your CPU and compiler must support AVX2 feature.
#
set(CONFIG_MDB_AVX2_KERNEL On)


# Enable building a kernel that is written in regular way ( plain C without intrinsics )
# and let your compiler handle all optimisations by itself.
# Performance of this kernel usually low, but it depends on how is your compiler doing his optimization job.
# When building this kernel try to enable all available CPU extension and fast math by specifying
# compiler flags like -march=native -ffast-math, etc.
# This kernel uses the same code as 'generic' kernel, but the difference is in
# for the generic kernel compiler disables all CPU extensions sse,avx,fma,etc and builds it
# with using x87 coprocessor math instead of default for x86-64 - sse math.
# Thus 'native' kernel performance is an example of how compiler can optimise code for a specific CPU.
# But both kernels shares other compiler flags like -O3 -ffast-math, etc, so 'native' kernel
# shows only how compiler uses vectorisation for optimisations.
#
set(CONFIG_MDB_NATIVE_KERNEL On)


# Enable building a kernel that is written in x86 assembly using NASM
# This kernel requires NASM compiller available in /usr/bin/nasm
# and CPU with AVX2,FMA support.
#
set(CONFIG_MDB_AVX_FMA_ASM_KERNEL Off)

#======================================================
# Render parameters                                   #
#======================================================

# Enable building render engine based on OpenGL 4.4.
# The main advantage of using this engine is persistent buffer mapping
# available since version 4.4 of OpenGL.
# Persistent buffer mapping reduces overhead on transfering memory from
# CPU to GPU.
# It potentyaly gives better performance on CPU kernels.
#
set(CONFIG_OGL_RENDER On)


# Enable OpenGL debug output if build type is Debug
# Requires GL_ARB_debug_output
#
set(CONFIG_OGL_DEBUG_OUTPUT On)
