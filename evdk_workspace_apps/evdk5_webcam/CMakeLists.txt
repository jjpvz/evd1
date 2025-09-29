cmake_minimum_required(VERSION 3.10)

project(evdk5_webcam)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(evdk5_webcam main.cpp)

target_link_libraries(evdk5_webcam ${OpenCV_LIBS})
