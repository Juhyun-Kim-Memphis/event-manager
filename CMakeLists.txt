cmake_minimum_required(VERSION 2.8)

project(event_manager)

set(gtest_SOURCE_DIR /home/junghwan/git/googletest)
set(BOOST_HOME /usr/local/)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${BOOST_HOME})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(SOURCE_FILES Task.cpp Task.hpp tests/basic.cpp tests/test_main.cpp Lock.hpp Pipe.hpp Worker.hpp Event.hpp Lock.cpp tests/event.cpp tests/pipe.cpp tests/lock.cpp tests/worker.cpp)

link_directories(${gtest_SOURCE_DIR})
link_directories(${BOOST_HOME}/lib)

add_executable(event_manager ${SOURCE_FILES})

target_link_libraries(event_manager gtest gtest_main boost_serialization)
