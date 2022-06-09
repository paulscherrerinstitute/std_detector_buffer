cmake_minimum_required(VERSION 3.17)

find_package(GTest REQUIRED)
add_executable(${PROJECT_NAME}_tests)

target_sources(${PROJECT_NAME}_tests
    PRIVATE
        test_bitshuffle.cpp
        test_RamBuffer.cpp
)

target_link_libraries(${PROJECT_NAME}_tests
    PRIVATE
        ${PROJECT_NAME}
        detectors
        bitshuffle::bitshuffle
        GTest::GTest
        std_detector_buffer::settings
)
