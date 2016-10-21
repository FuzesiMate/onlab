cmake_minimum_required(VERSION 3.4)
include(ExternalProject)

SET(PROJECT_NAME ComputerVision)
project(${PROJECT_NAME})


include_directories(${CMAKE_SOURCE_DIR}/ComputerVision/src)
include_directories(C:/include)
link_directories(C:/lib)

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/ComputerVision/src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

SET(DEBUG_LIBS opencv_core310d opencv_highgui310d opencv_calib3d310d opencv_videoio310d opencv_imgproc310d opencv_aruco310d xiapi32 libboost_system-vc140-mt-gd-1_62 tbb_debug tbbmalloc_debug tbbmalloc_proxy_debug libzmq_debug ws2_32)
SET(RELEASE_LIBS opencv_core310 opencv_highgui310 opencv_calib3d310 opencv_videoio310 opencv_imgproc310 opencv_aruco310 xiapi32 libboost_system-vc140-mt-1_62 tbb tbbmalloc tbbmalloc_proxy libzmq ws2_32)

foreach (_dlib ${DEBUG_LIBS})
	target_link_libraries(${PROJECT_NAME} debug ${_dlib})
endforeach()

foreach (_rlib ${RELEASE_LIBS})
	target_link_libraries(${PROJECT_NAME} optimized ${_rlib})
endforeach()

#target_link_libraries(${PROJECT_NAME} debug ${DEBUG_LIBS} optimized ${RELEASE_LIBS} )
