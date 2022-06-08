cmake_minimum_required(VERSION 3.12)
project(std_detector_buffer CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG_OUTPUT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_OUTPUT")

set(HDF5_PREFER_PARALLEL ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(sdb_settings)
include(sdb_conan)

conan_config_install(ITEM ${CMAKE_HOME_DIRECTORY}/conan/settings.yml)
conan_cmake_autodetect(DETECTED_SETTINGS)
conan_cmake_install(
        PATH_OR_REFERENCE ${CMAKE_SOURCE_DIR}/conan
        BUILD missing
        SETTINGS ${DETECTED_SETTINGS}
        SETTINGS compiler.psi=True # ensure that all packages are built or used from local cache
)

include(CTest)
add_subdirectory("detectors")
add_subdirectory("core-buffer")
add_subdirectory("std-det-writer")
add_subdirectory("std-stream-send")
add_subdirectory("std-udp-recv")
add_subdirectory("std-udp-sync")
add_subdirectory(external)
