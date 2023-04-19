cmake_minimum_required(VERSION 3.17)
project(std_buffer_writer)

find_package(fmt REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(redis++ REQUIRED)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        src/buffer_writer.cpp
        src/buffer_writer.hpp
        src/redis_sender.cpp
        src/redis_sender.hpp
        src/writer.cpp
)

sdb_package(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        core_buffer::core_buffer
        utils::utils
        fmt::fmt
        ZeroMQ::ZeroMQ
        redis++::redis++
        std_daq_interface::std_daq_interface
        std_detector_buffer::settings
)

if(SDB_WITH_BSREAD)
    include(GNUInstallDirs)
    install(FILES ${bsread_receiver_LIBS} TYPE LIB)
endif()
