cmake_minimum_required(VERSION 3.17)
project(std_daq_interface)

find_package(Protobuf REQUIRED)

add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PUBLIC
        std_daq/image_metadata.proto
        std_daq/writer_command.proto
)

protobuf_generate(
    TARGET ${PROJECT_NAME}
    LANGUAGE cpp
    PROTOC_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/generated)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        protobuf::libprotobuf
    PRIVATE
        std_detector_buffer::settings
)
