cmake_minimum_required(VERSION 3.12)

if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    add_executable(detectors_tests)
    target_sources(detectors_tests
        PRIVATE
            test_gigafrost.cpp
    )
    target_link_libraries(detectors_tests
        PRIVATE
            detectors
            GTest::GTest
    )
endif()

