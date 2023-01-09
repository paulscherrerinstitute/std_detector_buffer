cmake_minimum_required(VERSION 3.17)
project(std_live_stream)

find_package(fmt REQUIRED)
find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(RapidJSON REQUIRED)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        src/live_stream.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        core_buffer::core_buffer
        detectors::detectors
        utils::utils
        fmt::fmt
        ZeroMQ::ZeroMQ
        Threads::Threads
        rt
        std_detector_buffer::settings
)

sdb_package(${PROJECT_NAME})