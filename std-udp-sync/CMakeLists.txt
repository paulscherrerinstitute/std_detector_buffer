cmake_minimum_required(VERSION 3.12)

find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(date REQUIRED)
find_package(RapidJSON REQUIRED)

add_library(std-udp-sync-lib)
target_include_directories(std-udp-sync-lib PUBLIC include/)
target_link_libraries(std-udp-sync-lib
        ZeroMQ::ZeroMQ
        RapidJSON::RapidJSON
        date::date
        core-buffer-lib)

target_sources(std-udp-sync-lib
    PRIVATE
        src/SyncStats.cpp
        src/ZmqPulseSyncReceiver.cpp
)

add_executable(std-udp-sync src/main.cpp)

set_target_properties(std-udp-sync PROPERTIES OUTPUT_NAME std_udp_sync)
target_link_libraries(std-udp-sync
        core-buffer-lib
        std-udp-sync-lib
        ZeroMQ::ZeroMQ
        Threads::Threads
        rt)
