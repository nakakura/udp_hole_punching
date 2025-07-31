cmake_minimum_required(VERSION 3.16)
project(udp_hole_punching VERSION 1.0.0)

# C++17を要求（STUN実装でstd::optionalなどを使用するため）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

include_directories(include)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_library(udp_hole_punching_lib ${SOURCES})

enable_testing()
add_subdirectory(test)

# examplesディレクトリがある場合のみ追加
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt)
    add_subdirectory(examples)
endif()
