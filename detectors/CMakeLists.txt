
add_library(detectors INTERFACE)
target_include_directories(core-buffer-lib PUBLIC .)

enable_testing()
add_subdirectory(test/)