cmake_minimum_required(VERSION 3.17)
project(std_data_chunker)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ucp
        uct
        ucs
        ucm
        std_detector_buffer::settings
)

sdb_package(${PROJECT_NAME})
