cmake_minimum_required(VERSION 3.7.2)
set(CMAKE_CXX_STANDARD 17)
project(chartisto)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wno-error=unused-parameter")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/date/include/date)
add_subdirectory(date)
add_subdirectory(chart)
add_subdirectory(test)
add_subdirectory(chartisto)
add_subdirectory(robotrade)
