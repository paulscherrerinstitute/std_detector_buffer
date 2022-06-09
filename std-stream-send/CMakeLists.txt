cmake_minimum_required(VERSION 3.17)
find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(RapidJSON REQUIRED)

add_library(std-stream-send-lib)
target_include_directories(std-stream-send-lib PUBLIC include/)
target_link_libraries(std-stream-send-lib
        RapidJSON::RapidJSON
        ZeroMQ::ZeroMQ
        core_buffer::core_buffer)

target_sources(std-stream-send-lib
    PRIVATE
        src/ZmqLiveSender.cpp
)

add_executable(std-stream-send src/main.cpp)
set_target_properties(std-stream-send PROPERTIES OUTPUT_NAME std_stream_send)
target_link_libraries(std-stream-send
        core_buffer::core_buffer
        std-stream-send-lib
        ZeroMQ::ZeroMQ
        Threads::Threads
        rt)
