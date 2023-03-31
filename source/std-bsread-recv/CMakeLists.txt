cmake_minimum_required(VERSION 3.17)
project(std_bsread_recv)

find_package(bsread_receiver REQUIRED)
find_package(fmt REQUIRED)

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
        fmt::fmt
        std_detector_buffer::settings
)
