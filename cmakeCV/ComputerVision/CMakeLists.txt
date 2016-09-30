cmake_minimum_required(VERSION 3.4)
include(ExternalProject)

SET(PROJECT_NAME ComputerVision)
project(${PROJECT_NAME})


include_directories(${CMAKE_SOURCE_DIR}/ComputerVision/include)
include_directories(${OPENCV_INCLUDE})

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/ComputerVision/src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} m3api boost_system tbb tbbmalloc tbbmalloc_proxy)
