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
    common/platform.h
    utility/time.cpp
    utility/utility.h)

include_directories("3rdparty/include/")
#target_link_libraries("3rdparty/lib/libprotobuf.dylib")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "build")
add_executable(ADCore ${SOURCE_FILES})
add_definitions(-DMAC_CLANG)