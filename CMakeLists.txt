cmake_minimum_required(VERSION 3.8)

project(event_manager)

include_directories($ENV{GTEST_INC_PATH})
include_directories($ENV{BOOST_INC_PATH})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES Task.cpp Task.hpp tests/basic.cpp tests/test_main.cpp Lock.hpp Pipe.hpp Worker.hpp Event.hpp Lock.cpp tests/event.cpp tests/pipe.cpp tests/lock.cpp tests/worker.cpp Worker.cpp tests/test_utils.hpp tests/task.cpp)

link_directories($ENV{GTEST_LIB_PATH})
link_directories($ENV{BOOST_LIB_PATH})

add_executable(event_manager ${SOURCE_FILES})

target_link_libraries(event_manager gtest gtest_main boost_serialization pthread)
