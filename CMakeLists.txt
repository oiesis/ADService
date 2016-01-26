cmake_minimum_required(VERSION 3.3)
project(ADCore)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    common/types.h
    core_src/core.h
    core_src/core_main.cpp
    core_src/core_server.cpp
    unit_test/unit_test.cpp
    utility/cypher.cpp
    utility/hash.cpp
    utility/platform.h
    utility/time.cpp
    utility/utility.h)

add_executable(ADCore ${SOURCE_FILES})
add_definitions(-DMAC_CLANG)