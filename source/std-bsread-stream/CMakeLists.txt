cmake_minimum_required(VERSION 3.17)
project(std_bsread_stream)

find_package(fmt REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(hash-library REQUIRED)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        src/stream.cpp
)

sdb_package(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        core_buffer::core_buffer
        utils::utils
        fmt::fmt
        ZeroMQ::ZeroMQ
        hash-library::hash-library
        std_daq_interface::std_daq_interface
        std_detector_buffer::settings
)
