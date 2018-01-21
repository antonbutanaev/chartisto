cmake_minimum_required(VERSION 3.7.2)
set(CMAKE_CXX_STANDARD 14)
project(chartisto)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(chart)
add_subdirectory(test)
add_subdirectory(chartisto)
