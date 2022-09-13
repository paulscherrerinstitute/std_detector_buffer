cmake_minimum_required(VERSION 3.17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}")

if(NOT DEFINED ENV{CONAN_USER_HOME})
    set(ENV{CONAN_USER_HOME} ${CMAKE_HOME_DIRECTORY}/.conan_cache)
endif()

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD
            "https://github.com/conan-io/cmake-conan/raw/0.18.1/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake"
        EXPECTED_HASH SHA256=5cdb3042632da3efff558924eecefd580a0e786863a857ca097c3d1d43df5dcd
        TLS_VERIFY ON
    )
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)
