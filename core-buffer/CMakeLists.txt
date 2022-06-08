cmake_minimum_required(VERSION 3.12)
add_subdirectory(external)
project(core_buffer)

find_package(date REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(RapidJSON REQUIRED)

file(GLOB SOURCES
        src/*.cpp)

add_library(core-buffer-lib STATIC ${SOURCES})
target_include_directories(core-buffer-lib PUBLIC include/)
target_link_libraries(core-buffer-lib
    PRIVATE
        bitshuffle::bitshuffle
        date::date
        ZeroMQ::ZeroMQ
        RapidJSON::RapidJSON
        detectors
)

if (USE_EIGER)
        target_compile_definitions(core-buffer-lib PUBLIC -DUSE_EIGER)
endif (USE_EIGER)

if (DEBUG_OUTPUT)
        target_compile_definitions(core-buffer-lib PUBLIC -DDEBUG_OUTPUT)
endif(DEBUG_OUTPUT)

add_subdirectory(test)
