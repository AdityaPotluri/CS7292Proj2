cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)
project(pp_assign LANGUAGES C CXX)

message(STATUS "C Compiler: ${CMAKE_C_COMPILER_ID}")
message(STATUS "CXX Compiler: ${CMAKE_CXX_COMPILER_ID}")

set (CMAKE_CXX_STANDARD 17)
include_directories(${CMAKE_SOURCE_DIR}/include)

if (CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64|X86_64|amd64|AMD64)")

    message(STATUS "Building for Linux")
    message(STATUS "Arch: ${CMAKE_SYSTEM_PROCESSOR}")

    add_executable(histogram "src/histogram.c")
    target_compile_options(histogram PUBLIC -O3)

    add_executable(find_evsets "src/find_evsets.cpp" "src/allocator.cpp" "src/eviction_set.cpp" "src/victim.cpp" "src/x86_64_timer.cpp")
    target_compile_options(find_evsets PUBLIC -O3)
endif()
