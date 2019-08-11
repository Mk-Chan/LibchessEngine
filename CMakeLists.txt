cmake_minimum_required(VERSION 3.12)

project(LibchessEngine CXX)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

enable_testing()

add_executable(engine main.cpp evaluation.cpp evaluation.h search.h search.cpp tune.h)

target_link_libraries(engine Threads::Threads)