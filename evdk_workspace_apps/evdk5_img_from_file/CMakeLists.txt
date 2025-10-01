cmake_minimum_required(VERSION 3.10)

project(evdk5_img_from_file)

include_directories(../../evdk_operators)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(evdk5_img_from_file
../../evdk_operators/coding_and_compression.c
../../evdk_operators/fonts.c
../../evdk_operators/graphics_algorithms.c
../../evdk_operators/histogram_operations.c
../../evdk_operators/image_fundamentals.c
../../evdk_operators/mensuration.c
../../evdk_operators/morphological_filters.c
../../evdk_operators/noise.c
../../evdk_operators/nonlinear_filters.c
../../evdk_operators/segmentation.c
../../evdk_operators/spatial_filters.c
../../evdk_operators/spatial_frequency_filters.c
../../evdk_operators/transforms.c
main.cpp
)

target_link_libraries(evdk5_img_from_file ${OpenCV_LIBS})
