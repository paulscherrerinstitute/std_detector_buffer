cmake_minimum_required(VERSION 3.17)

find_package(GTest REQUIRED)

add_executable(${PROJECT_NAME}_tests)

target_sources(${PROJECT_NAME}_tests
    PRIVATE
        test_packet_udp_receiver.cpp
)

target_link_libraries(${PROJECT_NAME}_tests
    PRIVATE
        ${PROJECT_NAME}_lib
        core_buffer::core_buffer
        GTest::GTest
        std_detector_buffer::settings
)

sbd_add_uts(${PROJECT_NAME}_tests)

find_package(RapidJSON REQUIRED)
find_package(fmt REQUIRED)

add_executable(std_udp_sim_jf simulator_jf.cpp)
target_link_libraries(std_udp_sim_jf
    PRIVATE
        core_buffer::core_buffer
        detectors::detectors
        RapidJSON::RapidJSON
        fmt::fmt
        std_detector_buffer::settings
)
