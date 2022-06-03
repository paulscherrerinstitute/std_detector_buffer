add_library(detectors INTERFACE)
target_include_directories(detectors INTERFACE .)

enable_testing()
add_subdirectory(test/)
