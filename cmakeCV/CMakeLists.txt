cmake_minimum_required(VERSION 3.0)
project(cvapplication)

add_definitions(-std=c++11)

include_directories("src/")
include_directories("C:/opencv/build2/install/include/")
include_directories("C:/tbb44_20160316oss/include/")
include_directories("C:/boost/boost_1_55_0/")

file(GLOB SOURCES "src/*.cpp")
add_executable(cvapplication ${SOURCES})

link_directories("C:/opencv/build2/install/x86/mingw/lib/")
link_directories("C:/XIMEA/API/x86/")
link_directories("C:/tbb44_20160316oss/build/windows_ia32_gcc_mingw4.8.1_release/")

target_link_libraries(cvapplication tbb tbbmalloc tbbmalloc_proxy m3api)
target_link_libraries(cvapplication libopencv_core310 libopencv_highgui310 libopencv_imgcodecs310 libopencv_imgproc310 libopencv_videoio310 libopencv_aruco310 libopencv_calib3d310)
