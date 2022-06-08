cmake_minimum_required(VERSION 3.17)
project(detectors)

add_library(detectors INTERFACE)
target_include_directories(detectors INTERFACE .)

add_subdirectory(test)
