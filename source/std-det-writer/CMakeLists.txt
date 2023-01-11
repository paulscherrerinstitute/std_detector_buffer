cmake_minimum_required(VERSION 3.17)
project(std_det_writer)

find_package(MPI REQUIRED)
find_package(date REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(HDF5 REQUIRED)

add_library(${PROJECT_NAME}_lib)
add_library(${PROJECT_NAME}::${PROJECT_NAME}_lib ALIAS ${PROJECT_NAME}_lib)

target_include_directories(${PROJECT_NAME}_lib PUBLIC include)

target_link_libraries(${PROJECT_NAME}_lib
    PUBLIC
        RapidJSON::RapidJSON
        hdf5::hdf5
    PRIVATE
        core_buffer::core_buffer
        bitshuffle::bitshuffle
        MPI::MPI_CXX
        std_detector_buffer::settings
)

target_sources(${PROJECT_NAME}_lib
    PRIVATE
        src/JFH5Writer.cpp
        src/WriterStats.cpp
)

add_executable(${PROJECT_NAME} src/main.cpp)
sdb_package(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_NAME}_lib
        core_buffer::core_buffer
        date::date
        ZeroMQ::ZeroMQ
        RapidJSON::RapidJSON
        utils::utils
        hdf5::hdf5
        MPI::MPI_CXX
        std_detector_buffer::settings
)
