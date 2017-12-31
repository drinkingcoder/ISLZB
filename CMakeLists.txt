cmake_minimum_required(VERSION 2.8)
project(zbuffer)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

find_package( OpenCV REQUIRED )
message(STATUS "OpenCV version: {$OpenCV_VERSION}")
if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found")
endif()

add_executable(main main.cpp Object.cpp IntervalScanLineZBuffer.cpp)
target_link_libraries( main ${OpenCV_LIBS} )
