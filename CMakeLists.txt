cmake_minimum_required(VERSION 3.14)
project(demo)

set(CMAKE_CXX_FLAGS "-g -O0 -fPIC")
if (ASYNC_TYPE STREQUAL SYNC)
    message(STATUS "build with SYNC")
    add_definitions(-DSYNC)
endif()

if (REPLY_TYPE STREQUAL GOOD_TYPE)
    message(STATUS "build with GOOD_TYPE")
    add_definitions(-DGOOD_TYPE)
endif()

if (BUILD_TYPE STREQUAL TSAN)
    message(STATUS "build with tsan")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
endif()

add_executable(a.out main.cpp)
target_link_libraries(a.out pthread libevent.a libevent_pthreads.a)
