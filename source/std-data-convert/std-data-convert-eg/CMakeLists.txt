cmake_minimum_required(VERSION 3.17)
project(std_data_convert_eg)

find_package(argparse REQUIRED)
find_package(fmt REQUIRED)
find_package(ZeroMQ REQUIRED)

add_library(${PROJECT_NAME}_lib)
add_library(${PROJECT_NAME}::${PROJECT_NAME}_lib ALIAS ${PROJECT_NAME}_lib)

target_include_directories(${PROJECT_NAME}_lib PUBLIC include)

target_link_libraries(${PROJECT_NAME}_lib
    PUBLIC
        detectors::detectors
    PRIVATE
        fmt::fmt
        utils::utils
        std_detector_buffer::settings
)

target_sources(${PROJECT_NAME}_lib
    PRIVATE
        src/converter.cpp
)

add_executable(${PROJECT_NAME} src/main.cpp)
sdb_package(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_NAME}_lib
        argparse::argparse
        core_buffer::core_buffer
        detectors::detectors
        fmt::fmt
        utils::utils
        ZeroMQ::ZeroMQ
        std_detector_buffer::settings
)
