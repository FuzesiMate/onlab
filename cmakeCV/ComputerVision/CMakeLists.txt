cmake_minimum_required(VERSION 3.4)
include(ExternalProject)

SET(PROJECT_NAME ComputerVision)
project(${PROJECT_NAME})


include_directories(${CMAKE_SOURCE_DIR}/ComputerVision/src)

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/ComputerVision/src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} opencv_core opencv_highgui opencv_calib3d opencv_videoio opencv_imgproc opencv_aruco m3api boost_system tbb tbbmalloc tbbmalloc_proxy zmq)
