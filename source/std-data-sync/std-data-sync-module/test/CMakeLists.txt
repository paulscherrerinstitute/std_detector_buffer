cmake_minimum_required(VERSION 3.17)

find_package(GTest REQUIRED)
find_package(range-v3 REQUIRED)
add_executable(${PROJECT_NAME}_tests)

target_sources(${PROJECT_NAME}_tests
    PRIVATE
        test_synchronizer.cpp
)

target_link_libraries(${PROJECT_NAME}_tests
    PRIVATE
        ${PROJECT_NAME}_lib
        GTest::GTest
        range-v3::range-v3
        std_detector_buffer::settings
)
sbd_add_uts(${PROJECT_NAME}_tests)

find_package(benchmark REQUIRED)
add_executable(${PROJECT_NAME}_bench)

target_sources(${PROJECT_NAME}_bench PRIVATE benchmark_syncer.cpp)

target_link_libraries(${PROJECT_NAME}_bench
    PRIVATE
        ${PROJECT_NAME}_lib
        benchmark::benchmark
        range-v3::range-v3
        std_detector_buffer::settings
)
