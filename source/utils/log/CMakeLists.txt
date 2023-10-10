cmake_minimum_required(VERSION 3.17)
project(log)

find_package(spdlog REQUIRED)
find_package(range-v3 REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)
add_library(utils::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS include
    FILES
        include/utils/log/logger.hpp
)

target_include_directories(${PROJECT_NAME} INTERFACE include)
target_link_libraries(${PROJECT_NAME} INTERFACE utils::version spdlog::spdlog range-v3::range-v3)
