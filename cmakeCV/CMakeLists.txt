cmake_minimum_required(VERSION 3.4)
SET(PROJECT_NAME cvApplication)
project(${PROJECT_NAME})

find_package(OpenCV REQUIRED)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")



include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} m3api boost_system tbb tbbmalloc tbbmalloc_proxy)
