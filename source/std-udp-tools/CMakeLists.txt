cmake_minimum_required(VERSION 3.17)
project(std_udp_tools)

find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(date REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)

add_executable(${PROJECT_NAME}_dump src/std_udp_dump.cpp)
sdb_package(${PROJECT_NAME}_dump)

target_link_libraries(${PROJECT_NAME}_dump
    PRIVATE
        ZeroMQ::ZeroMQ
        core_buffer::core_buffer
        utils::utils
        std_udp_recv::std_udp_recv_lib
        fmt::fmt
        rt
        std_detector_buffer::settings
)

add_executable(${PROJECT_NAME}_replay src/std_udp_replay.cpp)
target_link_libraries(${PROJECT_NAME}_replay
    PRIVATE
        ZeroMQ::ZeroMQ
        core_buffer::core_buffer
        utils::utils
        std_udp_recv::std_udp_recv_lib
        fmt::fmt
        rt
        std_detector_buffer::settings
)
