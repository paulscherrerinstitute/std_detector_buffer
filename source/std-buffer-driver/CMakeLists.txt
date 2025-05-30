cmake_minimum_required(VERSION 3.17)
project(std_buffer_driver)

find_package(fmt REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(Boost REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        src/main.cpp
        src/process_api_message.hpp
        src/process_api_message.cpp
        src/reader_state.hpp
        src/replay_settings.hpp
        src/state_manager.hpp
        src/replayer.hpp
        src/replayer.cpp
        src/socket_session.hpp
        src/socket_session.cpp
)

sdb_package(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        core_buffer::core_buffer
        utils::utils
        fmt::fmt
        Boost::boost
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        ZeroMQ::ZeroMQ
        std_buffer_common::std_buffer_common
        std_daq_interface::std_daq_interface
        std_detector_buffer::settings
)
