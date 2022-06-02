
add_library(detectors INTERFACE)
target_include_directories(detectors INTERFACE .)

SET(ERROR_MESSAGE "Please add \
        -DDETECTOR=[eiger, gigafrost, jungfrau, jfjoch]\
        option to CMake.")

# Detector selection logic. This is an option you have to set in CMake.
SET(DETECTOR "" CACHE STRING "Detector for which to build the DAQ")
if (DETECTOR STREQUAL "")
    message(FATAL_ERROR "Missing detector option.\n${ERROR_MESSAGE}")
endif()

if (DETECTOR STREQUAL "gigafrost")
    target_compile_definitions(detectors INTERFACE -DFOR_GIGAFROST)
elseif(DETECTOR STREQUAL "jungfrau")
    target_compile_definitions(detectors INTERFACE -DFOR_JUNGFRAU)
elseif(DETECTOR STREQUAL "eiger")
    target_compile_definitions(detectors INTERFACE -DFOR_EIGER)
elseif(DETECTOR STREQUAL "jfjoch")
    target_compile_definitions(detectors INTERFACE -DFOR_JFJOCH)
else()
    message(FATAL_ERROR "Unknown detector type ${DETECTOR}.\n${ERROR_MESSAGE}")
endif()

message("Building DAQ for detector ${DETECTOR}.")

enable_testing()
add_subdirectory(test/)
