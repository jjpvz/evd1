cmake_minimum_required(VERSION 3.10)

project(evdk5_unit_test)

include_directories(.)
include_directories(../../evdk_operators)
include_directories(Unity/src)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_compile_definitions(UNITY_INCLUDE_CONFIG_H)

add_executable(evdk5_unit_test
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
main.c
test_coding_and_compression.c
test_graphics_algorithms.c
test_histogram_operations.c
test_image_fundamentals.c
test_mensuration.c
test_morphological_filters.c
test_nonlinear_filters.c
test_segmentation.c
test_spatial_filters.c
test_spatial_frequency_filters.c
test_transforms.c
Unity/src/unity.c
)
