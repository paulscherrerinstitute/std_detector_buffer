cmake_minimum_required(VERSION 3.17)
project(std_data_convert)

find_package(fmt REQUIRED)
find_package(MPI REQUIRED)
find_package(HDF5 REQUIRED)

add_library(${PROJECT_NAME}_lib)
add_library(${PROJECT_NAME}::${PROJECT_NAME}_lib ALIAS ${PROJECT_NAME}_lib)

target_include_directories(${PROJECT_NAME}_lib PUBLIC include)

target_link_libraries(${PROJECT_NAME}_lib
    PRIVATE
        fmt::fmt
        MPI::MPI_CXX
        hdf5::hdf5
        std_detector_buffer::settings
)

target_sources(${PROJECT_NAME}_lib
    PRIVATE
        src/converter.cpp
        src/read_gains_and_pedestals.cpp
        src/stats_collector.cpp
)

add_executable(${PROJECT_NAME} src/main.cpp)
sdb_package(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_NAME}_lib
        fmt::fmt
        core_buffer::core_buffer
        detectors::detectors
        std_detector_buffer::settings
)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
