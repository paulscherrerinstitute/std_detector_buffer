cmake_minimum_required(VERSION 3.17)
project(std_stream_send)

find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(RapidJSON REQUIRED)

add_executable(${PROJECT_NAME}_binary src/binary/main.cpp)
target_include_directories(${PROJECT_NAME}_binary PUBLIC include)
target_link_libraries(${PROJECT_NAME}_binary
    PRIVATE

        core_buffer::core_buffer
        detectors::detectors
        ZeroMQ::ZeroMQ
        Threads::Threads
        rt
        std_detector_buffer::settings
)
