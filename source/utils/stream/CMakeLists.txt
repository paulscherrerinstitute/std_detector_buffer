cmake_minimum_required(VERSION 3.17)
project(stream)

find_package(hash-library REQUIRED)
find_package(fmt REQUIRED)

add_library(${PROJECT_NAME})
add_library(utils::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS include
    FILES
        include/utils/stream/bsread.hpp
        include/utils/stream/array10.hpp
)

target_sources(${PROJECT_NAME}
    PRIVATE
        src/array10.cpp
        src/bsread.cpp
        src/map_dtype_to_stream_type.hpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC include
    PRIVATE include/utils/stream
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        std_daq_interface::std_daq_interface
    PRIVATE
        hash-library::hash-library
        fmt::fmt
        std_detector_buffer::settings
)
