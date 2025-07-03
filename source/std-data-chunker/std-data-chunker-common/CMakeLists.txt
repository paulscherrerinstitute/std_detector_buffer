cmake_minimum_required(VERSION 3.17)
project(std_data_chunker_common)

add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        include/std_data_chunker_common/ucx_rdma_endpoint.hpp
        src/ucx_rdma_endpoint.cpp
)

sdb_package(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC include PRIVATE include/std_data_chunker_common)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        std_daq_interface::std_daq_interface
    PRIVATE
        ucp
        uct
        ucs
        ucm
        std_detector_buffer::settings
)
