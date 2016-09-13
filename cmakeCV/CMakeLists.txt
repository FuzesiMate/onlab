cmake_minimum_required(VERSION 3.4)
SET(PROJECT_NAME cvApp)
project(${PROJECT_NAME})

find_package(OpenCV REQUIRED)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories("src/")
include_directories("/home/robot/tbb44_20160316oss/include")

file(GLOB RECURSE SOURCES src/*.cpp)
add_executable(${PROJECT_NAME} src/cvApplication.cpp)

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} tbb tbbmalloc tbbmalloc_proxy m3api)
