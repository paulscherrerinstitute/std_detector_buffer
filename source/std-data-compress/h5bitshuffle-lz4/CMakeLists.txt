cmake_minimum_required(VERSION 3.17)
project(std_data_compress_h5bitshuffle_lz4)

find_package(fmt REQUIRED)
find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(OpenMP REQUIRED)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        src/compress.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        core_buffer::core_buffer
        bitshuffle::bitshuffle
        utils::utils
        OpenMP::OpenMP_CXX
        fmt::fmt
        std_daq_interface::std_daq_interface
        ZeroMQ::ZeroMQ
        Threads::Threads
        rt
        std_detector_buffer::settings
)

sdb_package(${PROJECT_NAME})
