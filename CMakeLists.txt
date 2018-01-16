cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

project(tungsten)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(tungsten tungsten.cpp ConfigFile.h)
target_link_libraries(tungsten ${OpenCV_LIBS})
