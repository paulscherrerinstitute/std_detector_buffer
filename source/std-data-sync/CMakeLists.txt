cmake_minimum_required(VERSION 3.17)
project(std_data_sync_gf)

find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(date REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(fmt REQUIRED)

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
        core_buffer::core_buffer
        detectors::detectors
        fmt::fmt
    PRIVATE
        ZeroMQ::ZeroMQ
        RapidJSON::RapidJSON
        date::date
        std_detector_buffer::settings
)

add_executable(${PROJECT_NAME} src/main.cpp)
sdb_package(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_NAME}_lib
        ZeroMQ::ZeroMQ
        RapidJSON::RapidJSON
        Threads::Threads
        rt
        std_detector_buffer::settings
)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
