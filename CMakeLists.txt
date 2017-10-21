cmake_minimum_required(VERSION 3.8)
project(event_manager)

include_directories($ENV{gtest_SOURCE_DIR}/include $ENV{gtest_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(SOURCE_FILES Task.cpp Task.hpp tests/basic.cpp tests/test_main.cpp Module.hpp Lock.hpp)

link_directories($ENV{gtest_SOURCE_DIR})

add_executable(event_manager ${SOURCE_FILES})

target_link_libraries(event_manager gtest gtest_main)
