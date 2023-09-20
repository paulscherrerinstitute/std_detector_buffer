cmake_minimum_required(VERSION 3.17)
project(std_bsread_recv)

find_package(bsread_receiver REQUIRED)
find_package(fmt REQUIRED)
find_package(ZeroMQ REQUIRED)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        src/receiver.cpp
)

sdb_package(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        bsread_receiver::bsread_receiver
        core_buffer::core_buffer
        utils::utils
        fmt::fmt
        ZeroMQ::ZeroMQ
        std_daq_interface::std_daq_interface
        std_detector_buffer::settings
)
