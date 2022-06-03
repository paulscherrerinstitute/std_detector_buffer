file(GLOB SOURCES src/common/*.cpp)

add_library(std-udp-recv-lib STATIC ${SOURCES})
target_include_directories(std-udp-recv-lib PUBLIC include/)
target_link_libraries(std-udp-recv-lib
        external
        core-buffer-lib)

add_executable(eiger src/main_eiger.cpp)
set_target_properties(eiger PROPERTIES OUTPUT_NAME std_udp_recv_eiger)
target_link_libraries(eiger std-udp-recv-lib zmq rt)

enable_testing()
add_subdirectory(test/)
