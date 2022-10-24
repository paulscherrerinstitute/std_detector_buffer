cmake_minimum_required(VERSION 3.17)
project(std_stream_send_gf)

find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(RapidJSON REQUIRED)

add_executable(${PROJECT_NAME} src/sender.cpp)
sdb_package(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        core_buffer::core_buffer
        detectors::detectors
        ZeroMQ::ZeroMQ
        Threads::Threads
        rt
        std_detector_buffer::settings
)
