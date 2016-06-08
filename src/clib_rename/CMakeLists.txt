cmake_minimum_required(VERSION 2.8.12)

project(clib_rename)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(clib_rename clib_rename.cpp)
target_link_libraries(clib_rename ${OpenCV_LIBS})
