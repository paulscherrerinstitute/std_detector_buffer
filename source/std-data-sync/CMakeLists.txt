cmake_minimum_required(VERSION 3.17)
project(std_udp_sync)

find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(date REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(fmt REQUIRED)

add_library(${PROJECT_NAME}_lib)
add_library(${PROJECT_NAME}::${PROJECT_NAME}_lib ALIAS ${PROJECT_NAME}_lib)

target_sources(${PROJECT_NAME}_lib
    PRIVATE
        src/SyncStats.cpp
        src/ZmqPulseSyncReceiver.cpp
)

target_include_directories(${PROJECT_NAME}_lib PUBLIC include)
target_link_libraries(${PROJECT_NAME}_lib
    PUBLIC
        core_buffer::core_buffer
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
        detectors::detectors
        fmt::fmt
        ZeroMQ::ZeroMQ
        RapidJSON::RapidJSON
        Threads::Threads
        rt
        std_detector_buffer::settings
)
