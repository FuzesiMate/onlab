cmake_minimum_required(VERSION 3.4)
SET(PROJECT_NAME ComputerVisionFrameWork)
project(${PROJECT_NAME})

option(BUILD_APPLICATION "Build a sample application" OFF)
option(BUILD_STEREOCALIBRATE "Build stereo calibrator application" OFF)

option(INCLUDE_DIR "Directory that contains all include files")
option(LIBRARY_DIR "Directory that contains all library files")

option(OPENCV_INCLUDE "OpenCV include directory")
option(BOOST_INCLUDE "Boost include directory")
option(ZEROMQ_INCLUDE "ZeroMQ include directory")
option(TBB_INCLUDE "Intel Threading Building Blocks include directory")

option(OPENCV_LIB "OpenCV include directory")
option(BOOST_LIB "Boost include directory")
option(ZEROMQ_LIB "ZeroMQ include directory")
option(TBB_LIB "Intel Threading Building Blocks include directory")


SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(${CMAKE_SOURCE_DIR}/ComputerVision/include)

if(INCLUDE_DIR)
	include_directories(${INCLUDE_DIR})
endif(INCLUDE_DIR)

if(OPENCV_INCLUDE)
	include_directories(${OPENCV_INCLUDE})
endif(OPENCV_INCLUDE)

if(BOOST_INCLUDE)
	include_directories(${BOOST_INCLUDE})
endif(BOOST_INCLUDE)

if(TBB_INCLUDE)
	include_directories(${TBB_INCLUDE})
endif(TBB_INCLUDE)

if(ZEROMQ_INCLUDE)
	include_directories(${ZEROMQ_INCLUDE})
endif(ZEROMQ_INCLUDE)


if(LIBRARY_DIR)
	link_directories(${LIBRARY_DIR})
endif(LIBRARY_DIR)

if(OPENCV_LIB)
	link_directories(${OPENCV_LIB})
endif(OPENCV_LIB)

if(BOOST_LIB)
	link_directories(${BOOST_LIB})
endif(BOOST_LIB)

if(TBB_LIB)
	link_directories(${TBB_LIB})
endif(TBB_LIB)

if(ZEROMQ_LIB)
	link_directories(${ZEROMQ_LIB})
endif(ZEROMQ_LIB)

add_subdirectory(ComputerVision)

if(BUILD_APPLICATION)
add_subdirectory(Application)
add_dependencies(Application ComputerVision)
endif(BUILD_APPLICATION)

if(BUILD_STEREOCALIBRATE)
add_subdirectory(StereoCalibrate)
endif(BUILD_STEREOCALIBRATE)

