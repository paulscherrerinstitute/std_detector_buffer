cmake_minimum_required(VERSION 3.17)
project(std_array10_recv)

find_package(fmt REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        src/receiver.cpp
)

sdb_package(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        core_buffer::core_buffer
        utils::utils
        fmt::fmt
        nlohmann_json::nlohmann_json
        ZeroMQ::ZeroMQ
        std_daq_interface::std_daq_interface
        std_detector_buffer::settings
)
