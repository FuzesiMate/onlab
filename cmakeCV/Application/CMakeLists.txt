cmake_minimum_required(VERSION 3.4)
SET(PROJECT_NAME Application)
project(${PROJECT_NAME})

file(GLOB MAIN_SOURCES ${CMAKE_SOURCE_DIR}/Application/src/*.cpp)

add_executable(${PROJECT_NAME} ${MAIN_SOURCES})

SET(CV_DEBUG "${CMAKE_BINARY_DIR}/ComputerVision/Debug/ComputerVision.lib")
SET(CV_RELEASE "${CMAKE_BINARY_DIR}/ComputerVision/Release/ComputerVision.lib")

target_link_libraries(${PROJECT_NAME} debug ${CV_DEBUG})
target_link_libraries(${PROJECT_NAME} optimized ${CV_RELEASE})

SET(DEBUG_LIBS opencv_core310d opencv_highgui310d opencv_calib3d310d opencv_videoio310d opencv_imgproc310d opencv_aruco310d xiapi32 libboost_system-vc140-mt-gd-1_62 tbb_debug tbbmalloc_debug tbbmalloc_proxy_debug libzmq_debug ws2_32)
SET(RELEASE_LIBS opencv_core310 opencv_highgui310 opencv_calib3d310 opencv_videoio310 opencv_imgproc310 opencv_aruco310 xiapi32 libboost_system-vc140-mt-1_62 tbb tbbmalloc tbbmalloc_proxy libzmq ws2_32)

foreach (_dlib ${DEBUG_LIBS})
	target_link_libraries(${PROJECT_NAME} debug ${_dlib})
endforeach()

foreach (_rlib ${RELEASE_LIBS})
	target_link_libraries(${PROJECT_NAME} optimized ${_rlib})
endforeach()
