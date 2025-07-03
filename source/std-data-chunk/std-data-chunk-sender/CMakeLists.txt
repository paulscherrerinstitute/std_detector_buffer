cmake_minimum_required(VERSION 3.17)
project(std_data_chunk_sender)

find_package(ZeroMQ REQUIRED)

add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        src/main.cpp
)

sdb_package(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        std_data_chunk::common
        std_daq_interface::std_daq_interface
    PRIVATE
        ZeroMQ::ZeroMQ
        std_detector_buffer::settings
)
