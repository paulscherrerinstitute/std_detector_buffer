cmake_minimum_required(VERSION 3.17)
project(stats)

find_package(fmt REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)
add_library(utils::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PUBLIC
        include/utils/stats/basic_stats_collector.hpp
        include/utils/stats/compression_stats_collector.hpp
        include/utils/stats/module_stats_collector.hpp
        include/utils/stats/stats_collector.hpp
        include/utils/stats/sync_stats_collector.hpp
)

target_include_directories(${PROJECT_NAME} INTERFACE include)
target_link_libraries(${PROJECT_NAME} INTERFACE fmt::fmt)
