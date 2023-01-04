cmake_minimum_required(VERSION 3.17)
project(std_stream_send_eg)

find_package(fmt REQUIRED)
find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(RapidJSON REQUIRED)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        src/sender.cpp
)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        core_buffer::core_buffer
        detectors::detectors
        fmt::fmt
        Threads::Threads
        utils::utils
        ZeroMQ::ZeroMQ
        rt
        std_detector_buffer::settings
)
sdb_package(${PROJECT_NAME})
