cmake_minimum_required(VERSION 3.17)
project(std_det_writer)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(HDF5 REQUIRED)

add_library(${PROJECT_NAME}_lib)
add_library(${PROJECT_NAME}::${PROJECT_NAME}_lib ALIAS ${PROJECT_NAME}_lib)

target_include_directories(${PROJECT_NAME}_lib PUBLIC include)

target_link_libraries(${PROJECT_NAME}_lib
    PUBLIC
        hdf5::hdf5
        std_daq_interface::std_daq_interface
    PRIVATE
        core_buffer::core_buffer
        spdlog::spdlog
        bitshuffle::bitshuffle
        std_detector_buffer::settings
)

target_sources(${PROJECT_NAME}_lib
    PRIVATE
        src/H5Writer.cpp
)

add_executable(${PROJECT_NAME} src/main.cpp)
sdb_package(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_NAME}_lib
        core_buffer::core_buffer
        fmt::fmt
        spdlog::spdlog
        ZeroMQ::ZeroMQ
        utils::utils
        hdf5::hdf5
        std_detector_buffer::settings
)
