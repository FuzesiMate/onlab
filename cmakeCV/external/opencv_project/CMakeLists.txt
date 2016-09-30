cmake_minimum_required(VERSION 3.4)
include(ExternalProject)

SET(PROJECT_NAME opencv_builder)
project(${PROJECT_NAME})

ExternalProject_Add(opencv
SOURCE_DIR "${CMAKE_SOURCE_DIR}/external/opencv_project/opencv"
PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
CMAKE_ARGS -D WITH_XIMEA=ON
CMAKE_ARGS -D OPENCV_EXTRA_MODULES_PATH=${CMAKE_SOURCE_DIR}/external/opencv_contrib/modules
CMAKE_ARGS -D CMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}
INSTALL_COMMAND make install
)

SET(OPENCV_LIBS ${CMAKE_CURRENT_BINARY_DIR}/opencv/lib)

message(STATUS "Opencv Binaries:" ${OPENCV_LIBS})

SET(OPENCV_INCLUDE ${CMAKE_CURRENT_BINARY_DIR}/opencv/include)

message(STATUS "Opencv Include:" ${OPENCV_INCLUDE})
