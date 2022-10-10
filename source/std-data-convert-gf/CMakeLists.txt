cmake_minimum_required(VERSION 3.17)
project(std_data_convert_gf)

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
        std_detector_buffer::settings
)

target_sources(${PROJECT_NAME}_lib
    PRIVATE
        src/converter.cpp
        src/stats_collector.cpp
)

add_executable(${PROJECT_NAME} src/main.cpp)
sdb_package(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_NAME}_lib
        core_buffer::core_buffer
        detectors::detectors
        fmt::fmt
        ZeroMQ::ZeroMQ
        std_detector_buffer::settings
)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
