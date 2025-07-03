cmake_minimum_required(VERSION 3.17)
project(std_data_chunker)

add_subdirectory(std-data-chunk-common)
add_subdirectory(std-data-chunk-sender)

find_package(ZeroMQ REQUIRED)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ucp
        uct
        ucs
        ucm
        ZeroMQ::ZeroMQ
        std_detector_buffer::settings
        std_daq_interface::std_daq_interface
)

sdb_package(${PROJECT_NAME})
