cmake_minimum_required(VERSION 3.17)
project(std_stream_receive_gf)

find_package(fmt REQUIRED)
find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(RapidJSON REQUIRED)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        src/receiver.cpp
        src/stats_collector.cpp
        src/stats_collector.hpp
        src/synchronizer.hpp
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
