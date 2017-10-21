cmake_minimum_required(VERSION 2.8)
project(event_manager)

include_directories($ENV{gtest_SOURCE_DIR}/include $ENV{gtest_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES task.cpp task.hpp tests/basic.cpp tests/test_main.cpp)

link_directories($ENV{gtest_SOURCE_DIR})

add_executable(event_manager ${SOURCE_FILES})

target_link_libraries(event_manager gtest gtest_main)

