cmake_minimum_required(VERSION 3.17)
project(std_chunk_sender)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(ZeroMQ REQUIRED)

add_executable(${PROJECT_NAME})
sdb_package(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
        PRIVATE
        src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        core_buffer::core_buffer
        bitshuffle::bitshuffle
        fmt::fmt
        spdlog::spdlog
        ZeroMQ::ZeroMQ
        utils::utils
        std_detector_buffer::settings
)
