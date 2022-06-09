cmake_minimum_required(VERSION 3.17)

if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    add_executable(std-udp-recv-tests)
    target_sources(std-udp-recv-tests
        PRIVATE
            test_FrameUdpReceiver.cpp
            test_PacketUdpReceiver.cpp
    )
    target_link_libraries(std-udp-recv-tests
        PRIVATE
            core_buffer::core_buffer
            std-udp-recv-lib
            GTest::GTest
    )
endif()

find_package(RapidJSON REQUIRED)

add_executable(std-udp-sim simulator.cpp)
set_target_properties(std-udp-sim PROPERTIES OUTPUT_NAME std_udp_sim)
target_link_libraries(std-udp-sim
        core_buffer::core_buffer RapidJSON::RapidJSON detectors::detectors
        )
