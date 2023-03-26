cmake_minimum_required(VERSION 3.19)
project(srs_gstreamer)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--copy-dt-needed-entries")
set(CMAKE_CXX_STANDARD 14)

#找到OpenCV并包含
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

##Gstreamer
include_directories(/usr/include/gstreamer-1.0/)
include_directories(/usr/include/glib-2.0/)
include_directories(/usr/lib/aarch64-linux-gnu/glib-2.0/include)

#spdlog
include_directories(include/spdlog)


include_directories(src)
#gstreamer lib
add_library(stream_push src/gs_pushstream.cpp)

add_executable(srs_gstreamer src/main.cpp)


target_link_libraries(srs_gstreamer  stream_push gstreamer-1.0 gstapp-1.0 gstpbutils-1.0 gobject-2.0 )

target_link_libraries(srs_gstreamer ${OpenCV_LIBS})
