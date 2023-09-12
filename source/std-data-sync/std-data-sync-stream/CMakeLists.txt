cmake_minimum_required(VERSION 3.17)
project(std_data_sync_stream)

find_package(fmt REQUIRED)
find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        src/sync_config.hpp
        src/synchronizer.hpp
        src/queue_len_stats_collector.hpp
        src/stream_sync.cpp
        src/synchronizer.cpp
)

sdb_package(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        fmt::fmt
        core_buffer::core_buffer
        detectors::detectors
        ZeroMQ::ZeroMQ
        Threads::Threads
        utils::utils
        std_daq_interface::std_daq_interface
        rt
        std_detector_buffer::settings
)
