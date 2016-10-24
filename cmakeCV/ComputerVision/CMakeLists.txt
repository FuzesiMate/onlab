cmake_minimum_required(VERSION 3.4)

SET(PROJECT_NAME ComputerVision)
project(${PROJECT_NAME})

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/ComputerVision/src/*.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

