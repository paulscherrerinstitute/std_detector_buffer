cmake_minimum_required(VERSION 3.17)

if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    add_executable(${PROJECT_NAME}_tests)
    target_sources(${PROJECT_NAME}_tests
        PRIVATE
            test_bitshuffle.cpp
            test_RamBuffer.cpp
    )
    target_link_libraries(${PROJECT_NAME}_tests
        PRIVATE
            core-buffer-lib
            detectors
            bitshuffle::bitshuffle
            GTest::GTest
    )
    add_test(
            NAME ${PROJECT_NAME}_uts
            COMMAND ${PROJECT_NAME}_tests
    )
endif()
