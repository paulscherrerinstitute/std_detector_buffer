cmake_minimum_required(VERSION 3.17)
project(std_live_stream)

find_package(fmt REQUIRED)
find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(hash-library REQUIRED)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        src/arguments.cpp
        src/arguments.hpp
        src/live_stream.cpp
        src/live_stream_stats_collector.hpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        core_buffer::core_buffer
        utils::utils
        fmt::fmt
        std_daq_interface::std_daq_interface
        ZeroMQ::ZeroMQ
        hash-library::hash-library
        Threads::Threads
        rt
        std_detector_buffer::settings
)

sdb_package(${PROJECT_NAME})
