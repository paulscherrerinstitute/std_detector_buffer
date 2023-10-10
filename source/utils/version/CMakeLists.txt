cmake_minimum_required(VERSION 3.17)
project(version)

add_library(${PROJECT_NAME} INTERFACE)
add_library(utils::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

configure_file(include/utils/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/include/utils/version.hpp)

target_sources(${PROJECT_NAME}
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS include
        FILES
            include/utils/version.hpp
)

target_include_directories(${PROJECT_NAME} INTERFACE include)
