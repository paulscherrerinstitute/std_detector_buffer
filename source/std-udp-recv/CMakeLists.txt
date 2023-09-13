cmake_minimum_required(VERSION 3.17)
project(std_udp_recv)

find_package(fmt REQUIRED)
find_package(ZeroMQ REQUIRED)

add_library(${PROJECT_NAME}_lib)
add_library(${PROJECT_NAME}::${PROJECT_NAME}_lib ALIAS ${PROJECT_NAME}_lib)

target_sources(${PROJECT_NAME}_lib
    PRIVATE
        src/packet_udp_receiver.cpp
)

target_include_directories(${PROJECT_NAME}_lib PUBLIC include)
target_link_libraries(${PROJECT_NAME}_lib
    PUBLIC
        core_buffer::core_buffer
        detectors::detectors
    PRIVATE
        ZeroMQ::ZeroMQ
        fmt::fmt
        std_detector_buffer::settings
)

add_executable(${PROJECT_NAME}_jf src/jungfrau/main.cpp)
sdb_package(${PROJECT_NAME}_jf)
target_link_libraries(${PROJECT_NAME}_jf
    PRIVATE
        ${PROJECT_NAME}_lib
        utils::utils
        fmt::fmt
        ZeroMQ::ZeroMQ
        core_buffer::core_buffer
        rt
        std_detector_buffer::settings
)

add_executable(${PROJECT_NAME}_gf src/gigafrost/main.cpp)
sdb_package(${PROJECT_NAME}_gf)
target_link_libraries(${PROJECT_NAME}_gf
        PRIVATE
        ${PROJECT_NAME}_lib
        utils::utils
        fmt::fmt
        ZeroMQ::ZeroMQ
        core_buffer::core_buffer
        rt
        std_detector_buffer::settings
        )

add_executable(${PROJECT_NAME}_eg src/eiger/main.cpp)
sdb_package(${PROJECT_NAME}_eg)
target_link_libraries(${PROJECT_NAME}_eg
    PRIVATE
        ${PROJECT_NAME}_lib
        utils::utils
        fmt::fmt
        ZeroMQ::ZeroMQ
        core_buffer::core_buffer
        rt
        std_detector_buffer::settings
)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
