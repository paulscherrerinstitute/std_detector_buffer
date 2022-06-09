find_package(date REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(RapidJSON REQUIRED)

file(GLOB SOURCES src/*.cpp src/eiger/EigerFrameUdpReceiver.cpp)

add_library(std-udp-recv-lib STATIC ${SOURCES})
target_include_directories(std-udp-recv-lib PUBLIC include/)
target_link_libraries(std-udp-recv-lib
    PUBLIC
        RapidJSON::RapidJSON
        detectors::detectors
    PRIVATE
        date::date
        ZeroMQ::ZeroMQ
        core_buffer::core_buffer
)

add_executable(eiger src/eiger/main.cpp)
set_target_properties(eiger PROPERTIES OUTPUT_NAME std_udp_recv_eiger)
target_link_libraries(eiger std-udp-recv-lib
        ZeroMQ::ZeroMQ
        core_buffer::core_buffer rt)

add_subdirectory(test/)
