cmake_minimum_required(VERSION 3.17)
project(core_buffer)

find_package(date REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(RapidJSON REQUIRED)

add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        src/buffer_utils.cpp
        src/ram_buffer.cpp
        src/communicator.cpp
        src/external_communicator.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC include PRIVATE include/${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        ZeroMQ::ZeroMQ
    PRIVATE
        bitshuffle::bitshuffle
        date::date
        RapidJSON::RapidJSON
        detectors::detectors
        std_detector_buffer::settings
)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
