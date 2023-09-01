cmake_minimum_required(VERSION 3.17)
project(std_buffer_common)

find_package(fmt REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(redis++ REQUIRED)
find_package(range-v3 REQUIRED)
find_package(c-blosc2 REQUIRED)

add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        include/std_buffer_common/buffer_handler.hpp
        include/std_buffer_common/redis_handler.hpp
        src/buffer_handler.cpp
        src/redis_handler.cpp
)

sdb_package(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC include PRIVATE include/std_buffer_common)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        c-blosc2::c-blosc2
        redis++::redis++
    PRIVATE
        fmt::fmt
        range-v3::range-v3
        std_daq_interface::std_daq_interface
        std_detector_buffer::settings
)
