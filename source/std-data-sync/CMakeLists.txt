cmake_minimum_required(VERSION 3.17)
project(std_data_sync)

find_package(date REQUIRED)
find_package(fmt REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)

add_library(${PROJECT_NAME}_lib)
add_library(${PROJECT_NAME}::${PROJECT_NAME}_lib ALIAS ${PROJECT_NAME}_lib)

target_sources(${PROJECT_NAME}_lib
    PRIVATE
        src/sync_stats.cpp
        src/synchronizer.cpp
)

target_include_directories(${PROJECT_NAME}_lib PUBLIC include)
target_link_libraries(${PROJECT_NAME}_lib
    PUBLIC
        fmt::fmt
        core_buffer::core_buffer
        detectors::detectors
    PRIVATE
        date::date
        RapidJSON::RapidJSON
        ZeroMQ::ZeroMQ
        std_detector_buffer::settings
)

add_executable(${PROJECT_NAME}_module src/module_sync.cpp)
sdb_package(${PROJECT_NAME}_module)

target_include_directories(${PROJECT_NAME}_module PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME}_module
    PRIVATE
        ${PROJECT_NAME}_lib
        ZeroMQ::ZeroMQ
        RapidJSON::RapidJSON
        Threads::Threads
        utils::utils
        std_daq_interface::std_daq_interface
        rt
        std_detector_buffer::settings
)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
