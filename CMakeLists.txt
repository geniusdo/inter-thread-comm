cmake_minimum_required(VERSION 3.10)
project(ITC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-O2 -march=native)
endif()

option(BUILD_UNIT_TESTS "Build unit tests" ON)

if(BUILD_UNIT_TESTS)
    add_subdirectory(ut)
endif()