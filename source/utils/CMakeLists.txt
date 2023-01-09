cmake_minimum_required(VERSION 3.17)
project(utils)

find_package(fmt REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

configure_file(include/utils/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/include/utils/version.hpp)

target_sources(${PROJECT_NAME}
    INTERFACE
        include/utils/basic_stats_collector.hpp
        include/utils/module_stats_collector.hpp
        include/utils/stats_collector.hpp
        include/utils/version.hpp
)

target_include_directories(${PROJECT_NAME} INTERFACE include)
target_link_libraries(${PROJECT_NAME}
    INTERFACE
        fmt::fmt
        std_detector_buffer::settings
)
