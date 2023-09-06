cmake_minimum_required(VERSION 3.17)
project(std_stream_send)

find_package(fmt REQUIRED)
find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        src/sender.cpp
        src/sender_stats_collector.hpp
)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        core_buffer::core_buffer
        fmt::fmt
        std_daq_interface::std_daq_interface
        Threads::Threads
        utils::utils
        ZeroMQ::ZeroMQ
        rt
        std_detector_buffer::settings
)
sdb_package(${PROJECT_NAME})
