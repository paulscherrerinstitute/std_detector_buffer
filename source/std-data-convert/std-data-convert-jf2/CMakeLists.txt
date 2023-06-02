cmake_minimum_required(VERSION 3.17)
project(std_data_convert_jf2)

find_package(argparse REQUIRED)
find_package(fmt REQUIRED)
find_package(ZeroMQ REQUIRED)

add_executable(${PROJECT_NAME})
sdb_package(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        src/converter.cpp
        src/converter.hpp
        src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        argparse::argparse
        core_buffer::core_buffer
        detectors::detectors
        fmt::fmt
        utils::utils
        ZeroMQ::ZeroMQ
        std_detector_buffer::settings
)
