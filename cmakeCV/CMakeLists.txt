cmake_minimum_required(VERSION 3.4)
SET(PROJECT_NAME cvApplication)
project(${PROJECT_NAME})

add_subdirectory(external/opencv_project)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -L${OPENCV_LIBS}")

add_subdirectory(ComputerVision)


