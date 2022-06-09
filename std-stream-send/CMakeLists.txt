cmake_minimum_required(VERSION 3.17)
project(std_stream_send)

find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(RapidJSON REQUIRED)

add_library(${PROJECT_NAME}_lib)
add_library(${PROJECT_NAME}::${PROJECT_NAME}_lib ALIAS ${PROJECT_NAME}_lib)

target_include_directories(${PROJECT_NAME}_lib PUBLIC include)
target_link_libraries(${PROJECT_NAME}_lib
    PUBLIC
        core_buffer::core_buffer
    PRIVATE
        RapidJSON::RapidJSON
        ZeroMQ::ZeroMQ
        std_detector_buffer::settings
)

target_sources(${PROJECT_NAME}_lib
    PRIVATE
        src/ZmqLiveSender.cpp
)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_NAME}_lib
        core_buffer::core_buffer
        ZeroMQ::ZeroMQ
        Threads::Threads
        rt
        std_detector_buffer::settings
)
