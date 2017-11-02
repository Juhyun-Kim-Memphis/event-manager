cmake_minimum_required(VERSION 2.8)
project(event_manager)

set(gtest_SOURCE_DIR /home/juhyun/extern/googletest/googletest)

include_directories(/home/juhyun/extern/googletest/googletest/include /home/juhyun/extern/googletest/googletest)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(SOURCE_FILES Task.cpp Task.hpp tests/basic.cpp tests/test_main.cpp Module.hpp Lock.hpp Pipe.cpp Pipe.hpp Worker.hpp Event.hpp)

link_directories(/home/juhyun/extern/googletest/googletest)

add_executable(event_manager ${SOURCE_FILES})

target_link_libraries(event_manager gtest gtest_main)

