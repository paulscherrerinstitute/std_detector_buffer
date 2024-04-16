cmake_minimum_required(VERSION 3.17)
project(std_det_writer3)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(HDF5 REQUIRED)

add_executable(${PROJECT_NAME})
sdb_package(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        src/hdf5_file.cpp
        src/hdf5_file.hpp
        src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        core_buffer::core_buffer
        bitshuffle::bitshuffle
        fmt::fmt
        spdlog::spdlog
        ZeroMQ::ZeroMQ
        utils::utils
        hdf5::hdf5
        std_detector_buffer::settings
)
