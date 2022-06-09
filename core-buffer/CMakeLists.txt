cmake_minimum_required(VERSION 3.17)
project(core_buffer)

find_package(date REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(RapidJSON REQUIRED)

add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        src/BufferUtils.cpp
        src/RamBuffer.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        bitshuffle::bitshuffle
        date::date
        ZeroMQ::ZeroMQ
        RapidJSON::RapidJSON
        detectors::detectors
        std_detector_buffer::settings
)

if(USE_EIGER)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DUSE_EIGER)
endif()

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
