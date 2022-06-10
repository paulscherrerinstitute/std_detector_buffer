cmake_minimum_required(VERSION 3.17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}")

if(NOT DEFINED ENV{CONAN_USER_HOME})
    set(ENV{CONAN_USER_HOME} /home/root/.conan_cache)
endif()

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD
            "https://github.com/conan-io/cmake-conan/raw/0.17.0/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake"
        EXPECTED_HASH SHA256=3bef79da16c2e031dc429e1dac87a08b9226418b300ce004cc125a82687baeef
        TLS_VERIFY ON
    )
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)
