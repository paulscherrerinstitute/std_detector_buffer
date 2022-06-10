cmake_minimum_required(VERSION 3.17)
project(std_udp_recv)

find_package(date REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(RapidJSON REQUIRED)

add_library(${PROJECT_NAME}_lib)
add_library(${PROJECT_NAME}::${PROJECT_NAME}_lib ALIAS ${PROJECT_NAME}_lib)

target_sources(${PROJECT_NAME}_lib
    PRIVATE
        src/FrameStats.cpp
        src/FrameUdpReceiver.cpp
        src/PacketUdpReceiver.cpp
)

target_include_directories(${PROJECT_NAME}_lib PUBLIC include)
target_link_libraries(${PROJECT_NAME}_lib
    PUBLIC
        core_buffer::core_buffer
        detectors::detectors
        RapidJSON::RapidJSON
    PRIVATE
        date::date
        ZeroMQ::ZeroMQ
        std_detector_buffer::settings
)

add_executable(${PROJECT_NAME}_jf src/jungfrau/main.cpp)
target_link_libraries(${PROJECT_NAME}_jf
    PRIVATE
        ${PROJECT_NAME}_lib
        ZeroMQ::ZeroMQ
        core_buffer::core_buffer
        rt
        std_detector_buffer::settings
)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
