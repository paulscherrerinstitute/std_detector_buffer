cmake_minimum_required(VERSION 3.17)
project(std_stream_send_gf)

find_package(fmt REQUIRED)
find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(RapidJSON REQUIRED)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        src/sender.cpp
        src/stats_collector.cpp
        src/stats_collector.hpp
)
sdb_package(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        core_buffer::core_buffer
        detectors::detectors
        fmt::fmt
        ZeroMQ::ZeroMQ
        Threads::Threads
        rt
        std_detector_buffer::settings
)

add_executable(std_live_stream_gf)
target_sources(std_live_stream_gf
    PRIVATE
        src/live_stream.cpp
)

sdb_package(std_live_stream_gf)
target_include_directories(std_live_stream_gf PUBLIC include)
target_link_libraries(std_live_stream_gf
    PRIVATE
        core_buffer::core_buffer
        detectors::detectors
        fmt::fmt
        ZeroMQ::ZeroMQ
        Threads::Threads
        rt
        std_detector_buffer::settings
)
