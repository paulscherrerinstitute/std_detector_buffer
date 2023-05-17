cmake_minimum_required(VERSION 3.17)
project(std_buffer_writer)

find_package(fmt REQUIRED)
find_package(ZeroMQ REQUIRED)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        src/writer.cpp
)

sdb_package(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        core_buffer::core_buffer
        utils::utils
        fmt::fmt
        std_buffer_common::std_buffer_common
        ZeroMQ::ZeroMQ
        std_daq_interface::std_daq_interface
        std_detector_buffer::settings
)
