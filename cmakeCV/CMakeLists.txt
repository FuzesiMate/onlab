cmake_minimum_required(VERSION 3.4)
SET(PROJECT_NAME ComputerVisionFrameWork)
project(${PROJECT_NAME})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(${CMAKE_SOURCE_DIR}/ComputerVision/include)
include_directories(C:/include)
link_directories(C:/lib)

add_subdirectory(ComputerVision)
add_subdirectory(Application)

add_dependencies(Application ComputerVision)
