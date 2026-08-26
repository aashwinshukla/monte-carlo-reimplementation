cmake_minimum_required(VERSION 3.16)
project(RandomWalk CXX)
set(CMAKE_CXX_STANDARD 17)

add_executable(random-walk
    body.cpp
    walk.cpp
    validity-check.cpp
)

target_include_directories(random-walk PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
