cmake_minimum_required(VERSION 3.17)
project(utils)

find_package(argparse REQUIRED)
find_package(fmt REQUIRED)
find_package(RapidJSON REQUIRED)

add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

configure_file(include/utils/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/include/utils/version.hpp)

target_sources(${PROJECT_NAME}
    PUBLIC
        include/utils/args.hpp
        include/utils/basic_stats_collector.hpp
        include/utils/detector_config.hpp
        include/utils/module_stats_collector.hpp
        include/utils/stats_collector.hpp
        include/utils/sync_stats_collector.hpp
        include/utils/version.hpp
    PRIVATE
        src/detector_config.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC include PRIVATE include/utils)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        argparse::argparse
        fmt::fmt
        std_detector_buffer::settings
    PRIVATE
        RapidJSON::RapidJSON
)
