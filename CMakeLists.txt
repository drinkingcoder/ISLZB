cmake_minimum_required(VERSION 2.8)
project(zbuffer)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

find_package( OpenCV REQUIRED )

add_executable(main main.cpp Object.cpp IntervalScanLineZBuffer.cpp)
target_link_libraries( main ${OpenCV_LIBS} )
