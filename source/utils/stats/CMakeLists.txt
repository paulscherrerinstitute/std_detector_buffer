cmake_minimum_required(VERSION 3.17)
project(stats)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)
add_library(utils::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS include
        FILES
            include/utils/stats/compression_stats_collector.hpp
            include/utils/stats/module_stats_collector.hpp
            include/utils/stats/stats_collector.hpp
            include/utils/stats/sync_stats_collector.hpp
            include/utils/stats/timed_stats_collector.hpp
            include/utils/stats/queue_len_stats_collector.hpp
)

target_include_directories(${PROJECT_NAME} INTERFACE include)
target_link_libraries(${PROJECT_NAME} INTERFACE fmt::fmt spdlog::spdlog)
